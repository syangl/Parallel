#include<iostream>
#include<arm_neon.h>
#include <ctime>
#include <ratio>
#include <chrono>
using namespace std;

int32_t * key_slots_;
int32_t data_capacity_;

#define ALEX_DATA_NODE_KEY_AT key_slots_

template <class K>
inline int simd_search(int m, const K &key)
{
  int32x4_t keys = vmovq_n_s32(key);
  int32x4_t m_load;                
  int bound;
  if (ALEX_DATA_NODE_KEY_AT[m]>key)//预测位置大于key实际位置
  {
    for (int j = m-4; j>=0; j -= 4)
    {
      ////加载数组中的四个key到m_load向量
      m_load = vld1q_s32(ALEX_DATA_NODE_KEY_AT+j);
      //存放比较结果向量
      m_load = (int32x4_t)vceqq_s32(keys, m_load); 
      //变成整型的比较结果
      int32_t res = vaddvq_s32(m_load);
      
      if (res != 0)
      {////key在此向量中，确定其具体位置
        for (int i = j; i < j + 4; i++)
        {
          if (ALEX_DATA_NODE_KEY_AT[i]==key)
          {
            return bound = i;
          }
        }
      }
    }

    for (int i = 0; i < 3; i++)
    {
      if (ALEX_DATA_NODE_KEY_AT[i]==key)
      {
        return bound = i;
      }
    }
  }
  else
  {
    for (int j = m; j < data_capacity_; j += 4)
    {

      m_load = vld1q_s32(ALEX_DATA_NODE_KEY_AT + j);
      m_load = (int32x4_t)vceqq_s32(keys, m_load);
      int32_t res = vaddvq_s32( m_load);
      if (res != 0)
      {
        for (int i = j; i < j + 4; i++)
        {
          if (ALEX_DATA_NODE_KEY_AT[i]==key)
          {
            return bound = i;
          }
        }
      }
    }

    for (int i = data_capacity_ - 3; i < data_capacity_; i++)
    {
      if (ALEX_DATA_NODE_KEY_AT[i]==key)
      {
        return bound = i;
      }
    }
  }
  return -1;
}


template <class K>
inline int binary_search(int m, const K &key){
  int l ,r;
  if(ALEX_DATA_NODE_KEY_AT[m]>key){
    l = 0;
    r = m;
  }else{
    l = m;
    r = data_capacity_;
  }
  while (l < r) {
      int mid = l + (r - l) / 2;
      if (ALEX_DATA_NODE_KEY_AT[mid]>=key) {
        r = mid;
      } else {
        l = mid + 1;
      }
    }
    return l;
}

template <class K>
inline int search(int m, const K &key){
  if(ALEX_DATA_NODE_KEY_AT[m]>key){
    for(int i = 0; i < m; i++){
        if(ALEX_DATA_NODE_KEY_AT[i]==key){
          return i;
        }
    }
  }else{
    for(int i = m; i < data_capacity_; i++){
        if(ALEX_DATA_NODE_KEY_AT[i]==key){
          return i;
        }
    }
  }
  
}



int main(){//simd衍生子实验，密集数组测试，规模100M
  
  data_capacity_ = 100000000;
  ALEX_DATA_NODE_KEY_AT = new int[data_capacity_];
  for (int i = 0; i < 1; i++)
  {

    int test_key = 99900000, I = 99900000;//生成查找的目标键
    int start_pos = 9000;       //生成查找的起始位置
    cout<<"start pos"<<start_pos<<endl;
    for (int i = 0; i < data_capacity_; i++)
    {
      ALEX_DATA_NODE_KEY_AT[i] = i;
      if (i == I)
      {
        cout << " test_idex: " << i << endl;
        test_key = ALEX_DATA_NODE_KEY_AT[i];       //应该查到的正确键的索引
        cout << " test_key: " << test_key << endl; //应该查到的正确键
      }
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = std::chrono::high_resolution_clock::now(); //测试时间
    double per_search_time = 0.0;
    start_time = std::chrono::high_resolution_clock::now();
    int res_key = ALEX_DATA_NODE_KEY_AT[simd_search(start_pos, test_key)]; //实际查到的目标键值
    end_time = std::chrono::high_resolution_clock::now();
    per_search_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          end_time - start_time)
                          .count();
    cout << " simd_res_key: " << res_key << endl;
    cout << " simd_per_search_sec: " << per_search_time * 1e-9 << endl;

     auto start_time2 = std::chrono::high_resolution_clock::now();
    auto end_time2 = std::chrono::high_resolution_clock::now(); //测试时间
    double per_search_time2 = 0.0;
    start_time2 = std::chrono::high_resolution_clock::now();
    int res_key2 = ALEX_DATA_NODE_KEY_AT[binary_search(start_pos, test_key)]; //实际查到的目标键值
    end_time2 = std::chrono::high_resolution_clock::now();
    per_search_time2 = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          end_time2 - start_time2)
                          .count();
    cout << " bianry_res_key: " << res_key2 << endl;
    cout << " binary_per_search_sec: " << per_search_time2 * 1e-9 << endl;

    auto start_time3 = std::chrono::high_resolution_clock::now();
    auto end_time3 = std::chrono::high_resolution_clock::now(); //测试时间
    double per_search_time3 = 0.0;
    start_time3 = std::chrono::high_resolution_clock::now();
    int res_key3 = ALEX_DATA_NODE_KEY_AT[search(start_pos, test_key)]; //实际查到的目标键值
    end_time3 = std::chrono::high_resolution_clock::now();
    per_search_time3 = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          end_time3 - start_time3)
                          .count();
    cout << " res_key: " << res_key3 << endl;
    cout << " per_search_sec: " << per_search_time3 * 1e-9 << endl;
  }
  delete []ALEX_DATA_NODE_KEY_AT;
  return 0;
  
}
