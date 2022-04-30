
/**************************************pthread线程和锁开销测试程序***************************************/

#include<iostream>
#include<pthread.h>
#include<ctime>
#include<ratio>
#include<chrono>
#include<cmath>
#include<stdlib.h>
using namespace std;

const int N_SIZE = 1000;
const int ELEMENT_RANGE = 100;
const int THREAD_NUM = 1;
int** test_arr;

typedef struct{
  int t_id;
  int t_num;
  pthread_rwlock_t lock;
  void init_lock(){
      pthread_rwlock_init(&lock,NULL);
  }
}threadParam_t;

void* test_nolock(void* param){
    threadParam_t* p = (threadParam_t*)param;
    int id = p->t_id;
    int num = N_SIZE/p->t_num;
    auto start_time = std::chrono::high_resolution_clock::now();
    //cout<<"test_nolock enter  id "<< id <<" num "<<num<<endl;
    for(int i = id*num; i < (id+1)*num; i++){
        for(int j = 0; j < N_SIZE; j++){
            //cout<<"test_nolock for i = "<< i <<" j = "<< j <<endl;
            test_arr[i][j] = test_arr[i][j]*5+4;
        }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
    time = time/(num*N_SIZE);//每个线程一次加锁解锁的时间
    cout <<"test_nolock "<< id <<"           nsec: "<< time << "           sec: " << time * 1e-9 << endl;
    pthread_exit(NULL);
}

void* test_lock(void* param){
    threadParam_t * p = (threadParam_t*)param;
    int id = p->t_id;
    int num = N_SIZE/p->t_num;
    p->init_lock();
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int i = id*num; i < (id+1)*num; i++){
        for(int j = 0; j < N_SIZE; j++){
            pthread_rwlock_trywrlock(&p->lock);//加锁只为了测得加锁开销，此锁对功能没有任何实际意义
            test_arr[i][j] = test_arr[i][j]*5+4;
            pthread_rwlock_unlock(&p->lock);
        }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
    time = time/(num*N_SIZE);//每个线程一次加锁解锁的时间
    cout <<"test_lock "<< id <<"              nsec: "<< time << "           sec: " << time * 1e-9 << endl;
    pthread_exit(NULL);
}

/************************************************************main****************************************************/
int main(){
    //cout<<"main"<<endl;
    test_arr = new int*[N_SIZE];
    for(int i = 0; i < N_SIZE; i++){
        test_arr[i] = new int[N_SIZE];
    }
    //cout<<"init"<<endl;
    srand(time(NULL));
    //init test_arr
    //int count = 0;
    for(int i = 0 ; i < N_SIZE; i++){
        for(int j = 0; j < N_SIZE; j++){
            //count++;
            //cout<<count<<endl;
            test_arr[i][j] = rand()%ELEMENT_RANGE;
        }
    }
    //cout<<"nolcok test"<<endl;
    //nolcok test
    pthread_t* handles1 = (pthread_t*)malloc(THREAD_NUM*sizeof(pthread_t));// 创建对应的 Handle
    threadParam_t* param1 = (threadParam_t*)malloc(THREAD_NUM*sizeof(threadParam_t));
    auto start_time1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < THREAD_NUM; i++){
        param1[i].t_id = i;
        param1[i].t_num = THREAD_NUM;
        pthread_create(&handles1[i], NULL, test_nolock, (void*)param1);
    }
    for(int i = 0; i < THREAD_NUM; i++){
        pthread_join(handles1[i], NULL);
    }
    auto end_time1 = std::chrono::high_resolution_clock::now();
    auto time1 = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time1 - start_time1).count();
    cout <<"test_nolock_sum            nsec: "<< time1 << "           sec: " << time1 * 1e-9 << endl;

    //lock test
    pthread_t* handles2 = (pthread_t*)malloc(THREAD_NUM*sizeof(pthread_t));// 创建对应的 Handle
    threadParam_t* param2 = (threadParam_t*)malloc(THREAD_NUM*sizeof(threadParam_t));
    auto start_time2 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < THREAD_NUM; i++){
        param2[i].t_id = i;
        param2[i].t_num = THREAD_NUM;
        pthread_create(&handles2[i], NULL, test_lock, (void*)param2);
    }
    for(int i = 0; i < THREAD_NUM; i++){
        pthread_join(handles2[i], NULL);
    }
    auto end_time2 = std::chrono::high_resolution_clock::now();
    auto time2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time2 - start_time2).count();
    cout <<"test_lock_sum            nsec: "<< time2 << "           sec: " << time2 * 1e-9 << endl;

 



    for(int i = 0; i < N_SIZE; i++) 
        delete [] test_arr[i];
    delete [] test_arr;
    free(handles1);
    free(handles2);
    free(param1);
    free(param2);
    return 0;
}