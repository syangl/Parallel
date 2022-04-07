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

// Modify these if running your own workload
#define KEY_TYPE int //uint64_t // uint64_t, double
#define PAYLOAD_TYPE int //uint64_t

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
int main(int argc, char* argv[]) {

  auto flags = parse_flags(argc, argv);
  std::string keys_file_path = get_required(flags, "keys_file");
  std::string keys_file_type = get_required(flags, "keys_file_type");
  auto init_num_keys = stoi(get_required(flags, "init_num_keys"));
  auto total_num_keys = stoi(get_required(flags, "total_num_keys"));

  std::string lookup_distribution =
      get_with_default(flags, "lookup_distribution", "zipf");

  // Read keys from file
  auto keys = new KEY_TYPE[total_num_keys];
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
  std::mt19937_64 gen_payload(std::random_device{}());
  for (int i = 0; i < init_num_keys; i++) {
    values[i].first = keys[i];
    values[i].second = static_cast<PAYLOAD_TYPE>(gen_payload());
  }
  
  
  // Create ALEX and bulk load
  alex::Alex<KEY_TYPE, PAYLOAD_TYPE> index;

  std::sort(values, values + init_num_keys,
            [](auto const& a, auto const& b) { return a.first < b.first; });
  if(init_num_keys > 1){
    index.bulk_load(values, init_num_keys);
  }
  std::cout << "------------ bulkload finished ----------- " << std::endl;
  // Run workload
  int i = init_num_keys;
  long long insert_num_keys = total_num_keys - init_num_keys;

  std::cout << std::scientific;
  std::cout << std::setprecision(3);

  // Do inserts
  std::cout << "------------ start insert ----------- " << std::endl;
  auto insert_start_time = std::chrono::high_resolution_clock::now();
  for (; i < total_num_keys; i++) {
    index.insert(keys[i], static_cast<PAYLOAD_TYPE>(gen_payload()));
    //std::cout << "------------ inserted once ----------- " <<i-init_num_keys+1<< std::endl; /****/
  }
  auto insert_end_time = std::chrono::high_resolution_clock::now();
  double insert_time =
      std::chrono::duration_cast<std::chrono::nanoseconds>(insert_end_time -
                                                            insert_start_time)
          .count();
  std::cout << "insert_time: " << insert_time << std::endl;
  std::cout << "insert num is: " << insert_num_keys  << ", bw is " << insert_num_keys / insert_time * 1e9 << std::endl;

  std::cout << "------------ end insert --------------- " << std::endl;

  std::cout << "---------- start search (after insert) ---------------" << std::endl; 
  PAYLOAD_TYPE ret_search;
  PAYLOAD_TYPE* payload;
  auto read_start_time = std::chrono::high_resolution_clock::now();
  for (i = 0; i < total_num_keys; i++) {
     payload = index.get_payload(keys[i]);
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


  delete[] keys;
  delete[] values;
}
