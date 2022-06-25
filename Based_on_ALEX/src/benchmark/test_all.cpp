// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * Simple benchmark that runs a mixture of point lookups and inserts on ALEX.
 */

#include "../core/alex.h"

#include <iomanip>
#include <queue>
#include <time.h>
#include <algorithm> 

#include "flags.h"
#include "utils.h"

#include <pthread.h>
#include <stdlib.h>

#include <omp.h>

// Modify these if running your own workload
#define KEY_TYPE int//uint64_t // uint64_t, double
#define PAYLOAD_TYPE int//uint64_t

#define CPU_FREQ_MHZ (1994)
static inline unsigned long read_tsc(void) {
  unsigned long var;
  unsigned int hi, lo;

  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  var = ((unsigned long long int)hi << 32) | lo;

  return var;
}


/*
 * Required flags:
 * --keys_file              path to the file that contains keys
 * --keys_file_type         file type of keys_file (options: binary or text)
 * --init_num_keys          number of keys to bulk load with
 * --total_num_keys         total number of keys in the keys file
 * --batch_size             number of operations (lookup or insert) per batch
 *
 */


//PAYLOAD_TYPE ret_search;
//PAYLOAD_TYPE* payload;
/////////////////////////////////////////////////////////pthread////////////////////////////////////////
using namespace std;
alex::Alex<KEY_TYPE, PAYLOAD_TYPE> alex_index;
int total_num_keys;
int init_num_keys;
KEY_TYPE *keys;
std::mt19937_64 gen_payload(std::random_device{}());

int thread_num = 2;

typedef struct{
  int t_id;
  int t_num;
  
}threadParam_t;

void* thread(void* param){
  threadParam_t * p = (threadParam_t*)param;
  int id = p->t_id;
  int num = p->t_num;
  PAYLOAD_TYPE ret_search;
  PAYLOAD_TYPE* payload;
  
  //write
  auto t_start_time1 = std::chrono::high_resolution_clock::now();
  int start1 = id*((total_num_keys-init_num_keys)/num)+init_num_keys;
  int end1 = (id+1)*((total_num_keys-init_num_keys)/num)+init_num_keys;
  for (int i = start1; i < end1; i++) {
    alex_index.insert(keys[i], static_cast<PAYLOAD_TYPE>(gen_payload()));
  }
  //read
  int start2 = id*(total_num_keys/num);
  int end2 = (id+1)*(total_num_keys/num);
  for (int i = start2; i < end2; i++){
    payload = alex_index.get_payload(keys[i]);
    if(payload != nullptr){
      ret_search = *payload;
    }
  }
  auto t_end_time1 = std::chrono::high_resolution_clock::now();
  double t_time1 =
      std::chrono::duration_cast<std::chrono::nanoseconds>(t_end_time1 - t_start_time1).count();
  std::cout <<"id "<<id<< " t_search_time: " << t_time1 << " read num: "<<end2-start2<<" write num: "<<end1-start1<<std::endl;

  pthread_exit(NULL);
}


//pthread-sort

std::pair<KEY_TYPE, PAYLOAD_TYPE>* tmp_values;
std::pair<KEY_TYPE, PAYLOAD_TYPE>* values;

int	TRDNUM = 6;
#define		TASKNUM			(init_num_keys/TRDNUM)//整除
pthread_barrier_t trdbarr;
void* pthread_sort(void *arg); //多线程函数
void inside_sort(int left, int right); //快排
void merge(void); //合并函数

void* pthread_sort(void *arg){
	int index = (long)arg;
	inside_sort(index, index+TASKNUM-1);
	pthread_barrier_wait(&trdbarr);
	pthread_exit(NULL);
}

void inside_sort(int left, int right){
 
	if(left >= right){
		return ;
	}
 
	int i = left;
	int j = right;
	auto flag = tmp_values[left];
 
	while(i < j){
		while((j > i) && (tmp_values[j] >= flag)){
			j--;
		}
		if(j > i){
			tmp_values[i++] = tmp_values[j];
		}
		while((i < j) && (tmp_values[i] <= flag)){
			i++;
		}
		if(i < j){
			tmp_values[j--] = tmp_values[i];
		}
	}
	tmp_values[i] = flag;
 
	inside_sort(left, i);
	inside_sort(i+1, right);
}

void merge(){
 
	int index[TRDNUM];
	int i, sidx, minidx;
  std::pair<KEY_TYPE, PAYLOAD_TYPE> num; 
	
	for(i = 0; i < TRDNUM; i++){
		index[i] = i * TASKNUM;
	}
	for(sidx = 0; sidx < init_num_keys; sidx++){
    num.first = 2147483647 ;
    num.second = 2147483647 ;
		for(i = 0; i < TRDNUM; i++){
			if((index[i] < (i+1)*TASKNUM) && (tmp_values[index[i]] < num)){
				num = tmp_values[index[i]];
				minidx = i;
			}
		}
		values[sidx] = tmp_values[index[minidx]];
		index[minidx]++;
	}
}


////////////////////////////////////////////////////////////main//////////////////////////////////////////////////
int main(int argc, char* argv[]) {

 
  std::cin>>thread_num;
  std::cout<<"---thread num is :"<<thread_num<<std::endl;
  std::cin>>TRDNUM;
  std::cout<<"---sort thread num is :"<<TRDNUM<<std::endl;

  auto flags = parse_flags(argc, argv);
  std::string keys_file_path = get_required(flags, "keys_file");
  std::string keys_file_type = get_required(flags, "keys_file_type");
  /*auto*/ init_num_keys = stoi(get_required(flags, "init_num_keys"));
  /*auto*/ total_num_keys = stoi(get_required(flags, "total_num_keys"));

  std::string lookup_distribution =
      get_with_default(flags, "lookup_distribution", "zipf");

  // Read keys from file
  /*auto*/ keys = new KEY_TYPE[total_num_keys];
  if (keys_file_type == "binary") {
    load_binary_data(keys, total_num_keys, keys_file_path);
  } else if (keys_file_type == "text") {
    load_text_data(keys, total_num_keys, keys_file_path);
  } else {
    std::cerr << "--keys_file_type must be either 'binary' or 'text'"
              << std::endl;
    return 1;
  }

  std::random_shuffle(&keys[0], &keys[total_num_keys]);

  // Combine bulk loaded keys with randomly generated payloads
  /*auto*/ values = new std::pair<KEY_TYPE, PAYLOAD_TYPE>[init_num_keys];
  tmp_values = new std::pair<KEY_TYPE, PAYLOAD_TYPE>[init_num_keys];
  //std::mt19937_64 gen_payload(std::random_device{}());
  for (int i = 0; i < init_num_keys; i++) {
    tmp_values[i].first = keys[i];
    tmp_values[i].second = static_cast<PAYLOAD_TYPE>(gen_payload());
  }
  


  // Create ALEX and bulk load
  //alex::Alex<KEY_TYPE, PAYLOAD_TYPE> alex_index;
  //alex_index.lock_init();
  std::cout << "------------ bulkload start ----------- " << std::endl;

  pthread_t tid;
  pthread_barrier_init(&trdbarr, NULL, TRDNUM+1);
  auto sort_start = std::chrono::high_resolution_clock::now();
	for(int i = 0; i < TRDNUM; i++){
		pthread_create(&tid, NULL, pthread_sort, (void*)(TASKNUM*i));
	}
  pthread_barrier_wait(&trdbarr);
	merge();
  auto sort_end = std::chrono::high_resolution_clock::now(); //测试时间
  double sort_time = std::chrono::duration_cast<std::chrono::nanoseconds>(sort_end - sort_start).count();
  cout << "sort time: " << sort_time * 1e-9 <<"s"<< endl;
  pthread_barrier_destroy(&trdbarr);

  if(init_num_keys > 1){
    alex_index.bulk_load(values, init_num_keys);
  }

  std::cout << "------------ bulkload finished ----------- " << std::endl;
  // Run workload
#if PATTERN == 0
  int i = init_num_keys;//////////////////////////////////////////
#endif

  long long insert_num_keys = total_num_keys - init_num_keys;

  std::cout << std::scientific;
  std::cout << std::setprecision(3);

#if PATTERN == 0
  // Do inserts
  std::cout << "------------ start serial ----------- " << std::endl;

  auto serial_start_time = std::chrono::high_resolution_clock::now();
  for (; i < total_num_keys; i++)
  {
    alex_index.insert(keys[i], static_cast<PAYLOAD_TYPE>(gen_payload()));
  }
  // Do search
  
  PAYLOAD_TYPE ret_search;
  PAYLOAD_TYPE *payload;

  for (int i = 0; i < total_num_keys; i++)
  {
    payload = alex_index.get_payload(keys[i]);
    if (payload)
      ret_search = *payload;
  }
  auto serial_end_time = std::chrono::high_resolution_clock::now();
  double serial_time =
      std::chrono::duration_cast<std::chrono::nanoseconds>(serial_end_time -
                                                           serial_start_time)
          .count();
  std::cout << "serial_time: " << serial_time << std::endl;
  std::cout << " bw is " << (total_num_keys - 1) / serial_time * 1e9 << std::endl;
  std::cout << "---------- end serial ---------------" << std::endl;

#elif PATTERN == 1
  pthread_t *handles = (pthread_t *)malloc(thread_num * sizeof(pthread_t)); // 创建对应的 Handle
  threadParam_t *param = (threadParam_t *)malloc(thread_num * sizeof(threadParam_t));

  auto start_time = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < thread_num; i++)
  {
    param[i].t_id = i;
    param[i].t_num = thread_num;
    pthread_create(&handles[i], NULL, thread, (void *)&param[i]);
  }

  for (int i = 0; i < thread_num; i++)
  {
    pthread_join(handles[i], NULL);
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  double time =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
  std::cout << "time: " << time << std::endl;
  std::cout << "search num is: " << total_num_keys - 1 << ", bw is " << (total_num_keys - 1) / time * 1e9 << std::endl;

  //延迟删除
  while(!alex_index.delete_modelque.empty()){
    auto del_node = alex_index.delete_modelque.front();
    //delay_delete_model(del_node);
    alex_index.delete_modelque.pop();
  }
  while(!alex_index.delete_dataque.empty()){
    auto del_node = alex_index.delete_dataque.front();
    //delay_delete_data(del_node);
    alex_index.delete_dataque.pop();
  }
  free(handles);
  free(param);

#endif

  delete[] keys;
  delete[] values;
  delete[] tmp_values;
}



