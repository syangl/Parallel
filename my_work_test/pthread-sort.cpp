#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include<ctime>
#include<chrono>
using namespace std;
 
#define		TOTALSIZE		180000000
#define		THREADNUM			8
#define		TASKNUM			(TOTALSIZE/THREADNUM)
 
int arr[TOTALSIZE];
int arr_sorted[TOTALSIZE];
pthread_barrier_t barr;
 
void init_arr(); 
//多线程函数
void* pthread_sort(void *arg); 
//快排
void inside_sort(int left, int right); 
//合并
void merge(void); 
 
int main(){
	pthread_t tid;
 
	init_arr();
	pthread_barrier_init(&barr, NULL, THREADNUM+1);
    auto start_time2 = std::chrono::high_resolution_clock::now();
	for(int i = 0; i < THREADNUM; i++){
		pthread_create(&tid, NULL, pthread_sort, (void*)(TASKNUM*i));
	}
	pthread_barrier_wait(&barr);
	merge();

    auto end_time2 = std::chrono::high_resolution_clock::now(); //测试时间
    double time2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time2 - start_time2).count();
    cout << "sort time: " << time2 * 1e-9 <<"s"<< endl;
	
    pthread_barrier_destroy(&barr);

    for(int i = 1; i < 10; i++){
        cout<<arr_sorted[i]<<" ";
    }
 
	exit(0);
}
 
void* pthread_sort(void *arg){
	int index = (long)arg;
 
	inside_sort(index, index+TASKNUM-1);
	pthread_barrier_wait(&barr);
 
	pthread_exit(NULL);
}
 
void inside_sort(int left, int right){
 
	if(left >= right){
		return ;
	}
 
	int i = left;
	int j = right;
	int flag = arr[left];
 
	while(i < j){
		while((j > i) && (arr[j] >= flag)){
			j--;
		}
		if(j > i){
			arr[i++] = arr[j];
		}
		while((i < j) && (arr[i] <= flag)){
			i++;
		}
		if(i < j){
			arr[j--] = arr[i];
		}
	}
	arr[i] = flag;
 
	inside_sort(left, i);
	inside_sort(i+1, right);
}
 
void merge(){
 
	int index[THREADNUM];
	int sidx, num, minidx;
	
	for(int i = 0; i < THREADNUM; i++){
		index[i] = i * TASKNUM;
	}
	for(sidx = 0; sidx < TOTALSIZE; sidx++){
		num = 2147483647;
		for(int i = 0; i < THREADNUM; i++){
			if((index[i] < (i+1)*TASKNUM) && (arr[index[i]] < num)){
				num = arr[index[i]];
				minidx = i;
			}
		}
		arr_sorted[sidx] = arr[index[minidx]];
		index[minidx]++;
	}
}

void init_arr(){
	srand(time(NULL));
	for (int i = 0; i < TOTALSIZE; i++)
		arr[i] = rand() % 2147483647;
}
