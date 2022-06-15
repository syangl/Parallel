#include <iostream>
#include <cstdlib>
#include <time.h>
#include<chrono>
#include <iomanip>
#include <pthread.h>
#include <semaphore.h>
#include <xmmintrin.h> //SSE
#include <emmintrin.h> //SSE2
#include <pmmintrin.h> //SSE3
#include <tmmintrin.h> //SSSE3
#include <smmintrin.h> //SSE4.1
#include <nmmintrin.h> //SSSE4.2
#include <immintrin.h> //AVX、AVX2
#include <omp.h>
#include "mpi.h"
using namespace std;
#define MAX 100000

float** A;
int n;


void initArix(int n)
{
    for(int i=0;i<=n;i++)
    {
        for(int j=0;j<i;j++)
            A[i][j]=0.0;
        A[i][ i]=1.0;
        for(int j=i+1;j<=n;j++)
            A[i][j]=rand()%5;
    }
    for(int k=0;k<=n;k++)
        for(int i=k+1;i<=n;i++)
            for(int j=0;j<=n;j++)
                A[i][j]+=A[k][j];
}

void printArix(int n)
{
     for(int i = 1; i <= n ; i++)
    {
        for(int j = 1; j <= n; j++)
        {
            std::cout<<fixed<<setprecision(1)<<A[i][j]<<" ";
        }
        std::cout<<endl;
    }
}

/***normal***/
void normal(int n){
    for(int k = 1; k <= n; k++)
    {
        for (int j = k+1; j <= n; j++)
        {//simd
            A[k][j] = A[k][j]/A[k][k];
        }
        A[k][k] = 1.0;
        
        for (int i = k + 1; i <= n; i++){
            for (int j = k + 1; j <= n; j++)
            {//simd
                A[i][j] = A[i][j] - A[i][k]*A[k][j];
            }
            A[i][k] = 0.0;
        }
    }
}

int main(int argc, char *argv[])
{   
    n = 1000;
    A = new float*[MAX+1];
    for(int i = 0; i < MAX+1; i++)
    {
        A[i] = new float[MAX+1];
    }

    //simd
    __m128 vAkk;
    __m128 vAkj; 
    __m128 vAik;
    __m128 vAij;
    __m128 vt;

    int rank, size, provided;
    //MPI_Init(&argc, &argv);
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided < MPI_THREAD_MULTIPLE)
        MPI_Abort(MPI_COMM_WORLD, 1);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0)
    {//0号进程初始化
        srand(time(NULL));
        //init A
        //std::cout <<"---init matrix---"<<std::endl;
        initArix(n+1);
    }
    for (int i = 0; i <= n; i++)
    {//广播init A
        MPI_Bcast(&A[i][0],n+1,MPI_FLOAT,0,MPI_COMM_WORLD);
    }

    MPI_Status status;
    int r1, r2;//进程负责的矩阵子块的行号范围
    r1 = rank*(n - n%size)/size+1;
    if (rank != size - 1) 
    {
        r2 = rank*(n - n%size)/size + (n - n%size)/size;
    }else 
    {//可优化的点
        r2 = n;
    }
    std::cout <<"rank="<<rank<<" r1="<<r1<<" r2="<<r2<<std::endl;
    //消去
    auto t_start_time = std::chrono::high_resolution_clock::now();
    for(int k = 1; k <= n; k++)
    {
        if (r1 <= k && k <= r2)
        {
            vAkk =  _mm_set1_ps(A[k][k]);
            int j;
            for (j = k+1; j + 4 <= n; j+=4)
            {//simd
                vAkj = _mm_loadu_ps(A[k]+4);
                vAkj = _mm_div_ps(vAkj,vAkk);
                _mm_storeu_ps(A[k]+j, vAkj);
            }
            
            for(j = j-3; j <=n; j++)
            {
                A[k][j] = A[k][j]/A[k][k];
            }
            A[k][k] = 1.0;

            for (int j = rank+1; j < size; j++)
            {//除法结果发送给比自己进程号大的进程做消去
                MPI_Send(&A[k][0],n+1,MPI_FLOAT,j,j,MPI_COMM_WORLD);
            }
        }else if(k <= r2){
            MPI_Recv(&A[k][0],n+1,MPI_FLOAT,MPI_ANY_SOURCE,rank,MPI_COMM_WORLD,&status);
        }
        //对小于等于自己行号范围上界的做消去，大编号进程会继承前面的所有结果，最后一个进程保存最终结果
        //omp
        #pragma omp parallel for
        for (int i = k + 1; i <= r2; i++){
            vAik = _mm_set1_ps(A[k][k]);
            int j;
            for (j = k + 1; j + 4 <= r2; j+=4)
            {//simd
                vAkj =  _mm_loadu_ps(A[k]+j);
                vAij =  _mm_loadu_ps(A[i]+j);
                vAik =  _mm_loadu_ps(A[i]+j);
                vt = _mm_mul_ps(vAkj, vAik);
                vAij = _mm_sub_ps(vAij, vt);
                 _mm_storeu_ps(A[i]+j, vAij);
            }
            for(j = j - 3; j <=r2; j++)
            {
                A[k][j] = A[k][j]/A[k][k];
            }
            A[i][k] = 0.0;
        }
    }
    auto t_end_time = std::chrono::high_resolution_clock::now();
    double t_time =
       std::chrono::duration_cast<std::chrono::nanoseconds>(t_end_time - t_start_time).count();
    std::cout << "rank"<<rank<<"'s time: " << t_time*1e-9 <<" s"<< std::endl;

    MPI_Finalize();
    
    //std::cout<<"================================rank "<<rank<<"=================================="<<std::endl;
    if (rank==1)
        printArix(n);

    return 0;
}