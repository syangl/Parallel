#include <iostream>
#include <cstdlib>
#include <time.h>
#include<chrono>
#include <iomanip>
#include <pthread.h>
#include <semaphore.h>
#include <arm_neon.h>
using namespace std;
#define MAX 100000
/*** 1 is serial, 2 is simd 3 is pthread, 4 is openMP***/
#define PATTERN 3 

float** A;
int n;

void initMatrix(int n)
{
    for(int i=0;i<=n;i++)
    {
        for(int j=0;j<i;j++)
            A[i][j]=0.0;
        A[i][ i]=1.0;
        for(int j=i+1;j<=n;j++)
            A[i][j]=rand();
    }
    for(int k=0;k<=n;k++)
        for(int i=k+1;i<=n;i++)
            for(int j=0;j<=n;j++)
                A[i][j]+=A[k][j];
}

void printMatrix(int n)
{
     for(int i = 1; i <= n ; i++)
    {
        for(int j = 1; j <= n; j++)
        {
            cout<<fixed<<setprecision(1)<<A[i][j]<<" ";
        }
        cout<<endl;
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

/***simd***/
float32x4_t vAkk;
float32x4_t vAkj; 
float32x4_t vAik;
float32x4_t vAij;
float32x4_t vt;
void simd(int n)
{
    for(int k = 1; k <= n; k++)
    {
        vAkk = vdupq_n_f32(A[k][k]);
        int j;
        for (j = k+1; j + 4 <= n; j+=4)
        {//simd
            vAkj = vld1q_f32(A[k]+4);
            vAkj = vdivq_f32(vAkj,vAkk);
            vst1q_f32(A[k]+j, vAkj);
        }
        for(j = j - 3; j <=n; j++)
        {
            A[k][j] = A[k][j]/A[k][k];
        }
        A[k][k] = 1.0;
        
        for (int i = k + 1; i <= n; i++){
            int j;
            vAik = vdupq_n_f32(A[i][k]);
            for (j = k + 1; j + 4 <= n; j+=4)
            {//simd
                vAkj = vld1q_f32(A[k]+j);
                vAij = vld1q_f32(A[i]+j);
                vAik = vld1q_f32(A[i]+j);
                vt = vmulxq_f32(vAkj, vAik);
                vAij = vsubq_f32(vAij, vt);
                vst1q_f32(A[i]+j, vAij);
            }
            for(j = j - 3; j <=n; j++)
            {
                A[i][j] = A[i][j] - A[i][k]*A[k][j];
            }
            A[i][k] = 0.0;
        }
    }
}

/***pthread***/

/*** pthread 线程数 ***/
#define THREAD_NUM 4

typedef struct{
  int t_id;
}threadParam_t;

sem_t sem_main;
sem_t sem_workerstart[THREAD_NUM]; // 每个线程有自己专属的信号量
sem_t sem_workerend[THREAD_NUM];

void* thread(void* param){
    threadParam_t * p = (threadParam_t*)param;
    int id = p->t_id;

    float32x4_t vAkk;
    float32x4_t vAkj; 
    float32x4_t vAik;
    float32x4_t vAij;
    float32x4_t vt;

    auto t_start_time1 = std::chrono::high_resolution_clock::now();

    for (int k = 1; k <=n; k++)
    {
        sem_wait(&sem_workerstart[id]); 
        for (int i = k + 1 + id; i <= n; i+=THREAD_NUM)
        {
            int j;
            for (j = k + 1; j + 4 <= n; j+=4)
            {//simd
                vAkj = vld1q_f32(A[k]+j);
                vAij = vld1q_f32(A[i]+j);
                vAik = vld1q_f32(A[i]+j);
                vt = vmulxq_f32(vAkj, vAik);
                vAij = vsubq_f32(vAij, vt);
                vst1q_f32(A[i]+j, vAij);
            }
            for(j = j - 3; j <=n; j++)
            {
                A[i][j] = A[i][j] - A[i][k]*A[k][j];
            }
            A[i][k] = 0.0;
        }
        sem_post(&sem_main); 
        sem_wait(&sem_workerend[id]);
    }

    auto t_end_time1 = std::chrono::high_resolution_clock::now();
    double t_time1 =
        std::chrono::duration_cast<std::chrono::nanoseconds>(t_end_time1 - t_start_time1).count();
    std::cout << "---one_pth_time: " << t_time1*1e-9 << "s " << std::endl;

    pthread_exit(NULL);
}


















int main()
{
    srand(time(NULL));
    A = new float*[MAX+1];
    for(int i = 0; i < MAX+1; i++)
    {
        A[i] = new float[MAX+1];
    }
    n = 4;
    cin>>n;
    //init A
    initMatrix(n+1);
    
    //normal compute
    std::cout << "---pattern is "<<PATTERN<<"---"<<std::endl;

    auto t_start_time = std::chrono::high_resolution_clock::now();
    #if PATTERN == 1
    {//normal
        normal(n);
    }
    #elif PATTERN == 2
    {//simd
        simd(n);
    }
    #elif PATTERN == 3
    {//pthread
        //静态线程 信号量
        sem_init(&sem_main, 0, 0);
        for (int i = 0; i < THREAD_NUM; i++)
        {
            sem_init(&sem_workerend[i], 0, 0);
        }

        pthread_t handles[THREAD_NUM];// 创建对应的 Handle
        threadParam_t param[THREAD_NUM];// 创建对应的线程数据结构
        for (int i = 0; i < THREAD_NUM; i++)
        {
            param[i].t_id = i;
            pthread_create(&handles[i], NULL, thread, (void *)&param[i]);
        }
        
        //消去
        float32x4_t vAkk;
        float32x4_t vAkj;
        for (int k = 1; k <= n; k++)
        {
            vAkk = vdupq_n_f32(A[k][k]);
            int j;
            for (j = k+1; j + 4 <= n; j+=4)
            {//simd
                vAkj = vld1q_f32(A[k]+4);
                vAkj = vdivq_f32(vAkj,vAkk);
                vst1q_f32(A[k]+j, vAkj);
            }
            for(j = j - 3; j <=n; j++)
            {
                A[k][j] = A[k][j]/A[k][k];
            }
            A[k][k] = 1.0;

            for (int t_id = 0; t_id < THREAD_NUM; t_id++)
            {
                sem_post(&sem_workerstart[t_id]);
            }   

            for (int t_id = 0; t_id < THREAD_NUM; t_id++)
            {
                sem_wait(&sem_main);
            }

            for (int t_id = 0; t_id < THREAD_NUM; t_id++)
            {
                sem_post(&sem_workerend[t_id]);
            }
        }

        for (int i = 0; i < THREAD_NUM; i++)
        {
            pthread_join(handles[i], NULL);
        }

        sem_destroy(&sem_main);
        sem_destroy(sem_workerstart);
        sem_destroy(sem_workerend);


    }
    #elif
    {

    }
    #endif
    
    auto t_end_time = std::chrono::high_resolution_clock::now();
    double t_time =
       std::chrono::duration_cast<std::chrono::nanoseconds>(t_end_time - t_start_time).count();
    std::cout << "---time: " << t_time*1e-9 <<" s"<< std::endl;

    //result print
    printMatrix(n);

    return 0;
}