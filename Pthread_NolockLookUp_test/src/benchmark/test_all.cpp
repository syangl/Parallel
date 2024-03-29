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
/////////////////////////////////////////////////////////search-pthread////////////////////////////////////////
alex::Alex<KEY_TYPE, PAYLOAD_TYPE> alex_index;
int total_num_keys;
int init_num_keys;
KEY_TYPE *keys;
std::mt19937_64 gen_payload(std::random_device{}());

int thread_search_num = 4;//读线程总数
// int thread_write_num = 4;//写线程总数

typedef struct{
  int t_id;
  int t_num;
  
}threadParam_t;
//读线程
void *threadSearch(void *param){
  threadParam_t * p = (threadParam_t*)param;
  //int test_count = 0;
  int id = p->t_id;
  int num = p->t_num;
  PAYLOAD_TYPE ret_search;
  PAYLOAD_TYPE* payload;
  std::cout<<" threadSearch "<<id<<std::endl;

  auto t_start_time1 = std::chrono::high_resolution_clock::now();

  for (int i = id*(total_num_keys/num); i < (id+1)*(total_num_keys/num); i++){
    payload = alex_index.get_payload(keys[i]);
    //test_count++;
    if(payload != nullptr){
      ret_search = *payload;
    }
  }

     auto t_end_time1 = std::chrono::high_resolution_clock::now();
   double t_time1 =
       std::chrono::duration_cast<std::chrono::nanoseconds>(t_end_time1 -
                                                             t_start_time1)
           .count();
   std::cout << "t_search_time: " << t_time1 << " this_thread_count "<</*test_count<<*/std::endl;
  //std::cout<<" overread "<<id<<std::endl;
  pthread_exit(NULL);
}
//写线程
// void *threadWrite(void *param){
//   threadParam_t * p = (threadParam_t*)param;
//   int id = p->t_id;
//   int num = p->t_num;
//   PAYLOAD_TYPE ret_search;
//   PAYLOAD_TYPE* payload;

//   int test_count = 0;

//   std::cout<<" threadWrite "<<id<<std::endl;
  
//   auto t_start_time2 = std::chrono::high_resolution_clock::now();

//   for (int i = id*((total_num_keys-init_num_keys)/num)+init_num_keys; i < (id+1)*((total_num_keys-init_num_keys)/num)+init_num_keys; i++) {
//     //std::cout<<" insert_for "<<i-init_num_keys<<std::endl;
//     test_count++;
//     alex_index.insert(keys[i], static_cast<PAYLOAD_TYPE>(gen_payload()));
//   }

//   auto t_end_time2 = std::chrono::high_resolution_clock::now();
//    double t_time2 =
//        std::chrono::duration_cast<std::chrono::nanoseconds>(t_end_time2 -
//                                                              t_start_time2)
//            .count();
//   std::cout << "t_search_time: " << t_time2 << " this_thread_count "<<test_count<< std::endl;

//   //std::cout<<" overwrite "<<id<<std::endl;
//   pthread_exit(NULL);
// }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

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
  auto values = new std::pair<KEY_TYPE, PAYLOAD_TYPE>[init_num_keys];
  //std::mt19937_64 gen_payload(std::random_device{}());
  for (int i = 0; i < init_num_keys; i++) {
    values[i].first = keys[i];
    values[i].second = static_cast<PAYLOAD_TYPE>(gen_payload());
  }
  


  // Create ALEX and bulk load
  //alex::Alex<KEY_TYPE, PAYLOAD_TYPE> alex_index;
  //alex_index.lock_init();

  std::sort(values, values + init_num_keys,
            [](auto const& a, auto const& b) { return a.first < b.first; });
  if(init_num_keys > 1){
    alex_index.bulk_load(values, init_num_keys);
  }
  std::cout << "------------ bulkload finished ----------- " << std::endl;
  // Run workload

  int i = init_num_keys;//////////////////////////////////////////

  long long insert_num_keys = total_num_keys - init_num_keys;

  std::cout << std::scientific;
  std::cout << std::setprecision(3);

  // Do inserts
  std::cout << "------------ start insert ----------- " << std::endl;
  ////////////////////////////////////////////////////////pthread////////////////////////////////////////////////////////
  
  // pthread_t* handles1; 
  // handles1 = (pthread_t*)malloc(thread_write_num*sizeof(pthread_t));// 创建对应的 Handle
  // threadParam_t* param1;
  // param1 = (threadParam_t*)malloc(thread_write_num*sizeof(threadParam_t));
  
  // auto T1_start_time = std::chrono::high_resolution_clock::now();
  // for(int i = 0; i < thread_write_num; i++){
  //   param1[i].t_id = i;
  //   param1[i].t_num = thread_write_num; 
  //   pthread_create(&handles1[i], NULL, threadWrite, (void*)&param1[i]);
  // }

  // for(int i = 0; i < thread_write_num; i++){
  //   pthread_join(handles1[i], NULL);
  // }
  // auto T1_end_time = std::chrono::high_resolution_clock::now();
  // double insert_time =
  //      std::chrono::duration_cast<std::chrono::nanoseconds>(T1_end_time -
  //                                                            T1_start_time)
  //          .count();
  // std::cout << "T1_time: " << insert_time << std::endl;

  ///////////////////////////////////////////////////////pthread////////////////////////////////////////////////////////
  auto insert_start_time = std::chrono::high_resolution_clock::now();
  for (; i < total_num_keys; i++) {
    alex_index.insert(keys[i], static_cast<PAYLOAD_TYPE>(gen_payload()));
  }
  auto insert_end_time = std::chrono::high_resolution_clock::now();
  double insert_time =
      std::chrono::duration_cast<std::chrono::nanoseconds>(insert_end_time -
                                                            insert_start_time)
          .count();
  std::cout << "insert_time: " << insert_time << std::endl;



  std::cout << "insert num is: " << insert_num_keys  << ", bw is " << insert_num_keys / insert_time * 1e9 << std::endl;

  std::cout << "------------ end insert --------------- " << std::endl;

  std::cout << "---------- start Pthread search (after insert) ---------------" << std::endl;

  ////////////////////////////////////////////////////////pthread////////////////////////////////////////////////////////
  pthread_t* handles2; 
  handles2 = (pthread_t*)malloc(thread_search_num*sizeof(pthread_t));// 创建对应的 Handle
  threadParam_t* param2;
  param2 = (threadParam_t*)malloc(thread_search_num*sizeof(threadParam_t));

  auto T_start_time = std::chrono::high_resolution_clock::now();
  for(int i = 0; i < thread_search_num; i++){
    param2[i].t_id = i;
    param2[i].t_num = thread_search_num; 
    pthread_create(&handles2[i], NULL, threadSearch, (void*)&param2[i]);
  }
  
  for(int i = 0; i < thread_search_num; i++){
    pthread_join(handles2[i], NULL);
  }

  auto T_end_time = std::chrono::high_resolution_clock::now();
   double T_time =
       std::chrono::duration_cast<std::chrono::nanoseconds>(T_end_time -
                                                             T_start_time)
           .count();
   std::cout << "T_time: " << T_time << std::endl;
   std::cout << "search num is: " << total_num_keys - 1 << ", bw is " << (total_num_keys - 1) / T_time * 1e9 << std::endl;
   std::cout << "---------- end search (after insert) ---------------" << std::endl;

  ////////////////////////////////////////////////////////pthread////////////////////////////////////////////////////////
    std::cout << "---------- start search (after insert) ---------------" << std::endl;
  PAYLOAD_TYPE ret_search;
  PAYLOAD_TYPE* payload;
  auto read_start_time = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < total_num_keys; i++) {
     payload = alex_index.get_payload(keys[i]);
     if(payload)
       ret_search = *payload;

   }
   auto read_end_time = std::chrono::high_resolution_clock::now();
   double read_time =
       std::chrono::duration_cast<std::chrono::nanoseconds>(read_end_time -
                                                             read_start_time)
           .count();
   std::cout << "search_time: " << read_time << std::endl;
   std::cout << "search num is: " << total_num_keys - 1 << ", bw is " << (total_num_keys - 1) / read_time * 1e9 << std::endl;
   std::cout << "---------- end search (after insert) ---------------" << std::endl;


  //free(handles1);
  free(handles2);
  delete[] keys;
  delete[] values;
}
