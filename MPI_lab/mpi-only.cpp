#include <iostream>
#include <cstdlib>
#include <time.h>
#include<chrono>
#include <iomanip>
#include <pthread.h>
#include <semaphore.h>
#include <cmath>
//#include <arm_neon.h>
#include <omp.h>
#include "mpi.h"
using namespace std;
#define MAX 100000
/*** 1 is serial, 2 is simd 3 is pthread, 4 is openMP, 5 is mpi***/
#define PATTERN 5 
/*** 线程数 ***/
#define THREAD_NUM 4

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
            A[i][j]=rand()%50000;
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
    n = 16;
    A = new float*[MAX+1];
    for(int i = 0; i < MAX+1; i++)
    {
        A[i] = new float[MAX+1];
    }

    int rank, size;
    MPI_Init(&argc, &argv);
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
    
    int r1 = 0, r2 = 0;//进程负责的矩阵子块的行号范围
    //进程号越大的计算量越小，最后两个进程计算量相同
    if (rank == 0)
    {
        r1 = 1;
        r2 = n/2;
    }
    if (rank != 0 && rank != size -1)
    {
        for (int i = 1; i <= rank;i++)
        {
            r1 += n/pow(2.0,i);
        }
        r1 += 1;
        r2 = r1 - 1;
        r2 += n/pow(2.0,rank+1);
    }
    if (rank == size - 1)
    {
        for (int i = 1; i <= rank;i++)
        {
            r1 += n/pow(2.0,i);
        }
        r1 += 1;
        r2 = n;
    }
    
    //r1 = rank*(n - n%size)/size+1;
    // if (rank != size - 1) 
    // {
    //     r2 = rank*(n - n%size)/size + (n - n%size)/size;
    // }else 
    // {//可优化的点
    //     r2 = n;
    // }
    std::cout <<"rank="<<rank<<" r1="<<r1<<" r2="<<r2<<std::endl;
    //消去
    auto t_start_time = std::chrono::high_resolution_clock::now();
    for(int k = 1; k <= n; k++)
    {
        if (r1 <= k && k <= r2)
        {
            for (int j = k+1; j <= n; j++)
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
        for (int i = k+1; i <= r2; i++){
            for (int j = k + 1; j <= n; j++)
            {
                A[i][j] = A[i][j] - A[i][k]*A[k][j];
            }
            A[i][k] = 0.0;
        }
    }
    auto t_end_time = std::chrono::high_resolution_clock::now();
    double t_time =
       std::chrono::duration_cast<std::chrono::nanoseconds>(t_end_time - t_start_time).count();
    std::cout << "rank"<<rank<<"'s time: " << t_time*1e-9 <<" s"<< std::endl;

    //最后一个进程保存完成消去结果
    MPI_Finalize();
    if (rank == size-1)
         printArix(n);

    return 0;
}