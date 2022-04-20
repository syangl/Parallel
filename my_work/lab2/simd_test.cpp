#include<iostream>
#include<arm_neon.h>
#include<ctime>
#include<ratio>
#include<chrono>
#include<cmath>
using namespace std;

int32_t * key_slots_;
int32_t data_capacity_;

#define ALEX_DATA_NODE_KEY_AT key_slots_

///////////////////////////////simd//////////////////////////////////
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

/////////////////////////////////////binary////////////////////////////////////////////
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
/////////////////////////////////linear//////////////////////////////////////////////
template <class K>
inline int linear_search(int m, const K &key){
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

///////////////////////////////////binary_simd////////////////////////////
template <class K>
inline int binary_simd_search(int m, const K &key){
  int l ,r;
  int32x4_t keys = vmovq_n_s32(key);int32x4_t vec;
  if(ALEX_DATA_NODE_KEY_AT[m]>key){
    l = 0;
    r = m;
  }else{
    l = m;
    r = data_capacity_;
  }
  while (l < r) {
      int mid = l + (r - l) / 2;

      if(ALEX_DATA_NODE_KEY_AT[mid]==key){
        return mid;
      }

      if (ALEX_DATA_NODE_KEY_AT[mid]>key) {
        r = mid - 1;
        if(r>=3){
          vec = vld1q_s32(ALEX_DATA_NODE_KEY_AT+r-3);
          vec = (int32x4_t)vceqq_s32(keys, vec);
          int32_t res = vaddvq_s32(vec);
          if(res){
            for (int i = r-3; i <= r; i++)
            {
              if (ALEX_DATA_NODE_KEY_AT[i]==key)
              {
                return i;
              }
            }
          }
        }
      } else {
        l = mid + 1;
        if(l+3<data_capacity_){
          vec = vld1q_s32(ALEX_DATA_NODE_KEY_AT+l);
          vec = (int32x4_t)vceqq_s32(keys, vec);
          int32_t res = vaddvq_s32(vec);
          if(res){
            for (int i = l; i <= l+3; i++)
            {
              if (ALEX_DATA_NODE_KEY_AT[i]==key)
              {
                return i;
              }
            }
          }
        }
      }
  }
  return l;
}

////////////////////////////////expo_simd/////////////////////////////////////
template <class K>
inline int binary_simd_in_expo(int l, int r, const K &key){
  int32x4_t keys = vmovq_n_s32(key);int32x4_t vec;
  while (l < r) {
      int mid = l + (r - l) / 2;

      if(ALEX_DATA_NODE_KEY_AT[mid]==key){
        return mid;
      }

      if (ALEX_DATA_NODE_KEY_AT[mid]>key) {
        r = mid - 1;
        if(r>=3){
          vec = vld1q_s32(ALEX_DATA_NODE_KEY_AT+r-3);
          vec = (int32x4_t)vceqq_s32(keys, vec);
          int32_t res = vaddvq_s32(vec);
          if(res){
            for (int i = r-3; i <= r; i++)
            {
              if (ALEX_DATA_NODE_KEY_AT[i]==key)
              {
                return i;
              }
            }
          }
        }
      } else {
        l = mid + 1;
        if(l+3<data_capacity_){
          vec = vld1q_s32(ALEX_DATA_NODE_KEY_AT+l);
          vec = (int32x4_t)vceqq_s32(keys, vec);
          int32_t res = vaddvq_s32(vec);
          if(res){
            for (int i = l; i <= l+3; i++)
            {
              if (ALEX_DATA_NODE_KEY_AT[i]==key)
              {
                return i;
              }
            }
          }
        }
      }
  }
  return l;
}

  template <class K>
  inline int exponential_search_upper_bound_simd(int m, const K& key) {
    int bound = 1;
    int l, r;  // will do binary search in range [l, r)
    if (ALEX_DATA_NODE_KEY_AT[m]>key) {
      int size = m;
      while (bound < size &&
             ALEX_DATA_NODE_KEY_AT[m-bound]>key) {
        bound *= 2;
        //num_exp_search_iterations_++;s
      }
      l = m - std::min<int>(bound, size);
      r = m - bound / 2;
    } else {
      int size = data_capacity_ - m;
      while (bound < size &&
             ALEX_DATA_NODE_KEY_AT[m + bound]<=key) {
        bound *= 2;
        //num_exp_search_iterations_++;
      }
      l = m + bound / 2;
      r = m + std::min<int>(bound, size);
    }
    return binary_simd_in_expo(l, r, key);
  }

  ////////////////////////////////////////原版expo////////////////////////////////////
  template <class K>
  inline int binary_nonsimd_in_expo(int l, int r, const K &key){
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
  inline int exponential_search_upper_bound(int m, const K& key) {
    int bound = 1;
    int l, r;  // will do binary search in range [l, r)
    if (ALEX_DATA_NODE_KEY_AT[m]>key) {
      int size = m;
      while (bound < size &&
             ALEX_DATA_NODE_KEY_AT[m-bound]>key) {
        bound *= 2;
        //num_exp_search_iterations_++;
      }
      l = m - std::min<int>(bound, size);
      r = m - bound / 2;
    } else {
      int size = data_capacity_ - m;
      while (bound < size &&
             ALEX_DATA_NODE_KEY_AT[m + bound]<=key) {
        bound *= 2;
        //num_exp_search_iterations_++;
      }
      l = m + bound / 2;
      r = m + std::min<int>(bound, size);
    }
    return binary_nonsimd_in_expo(l, r, key);
  }


//////////////////////////////////////////////////////////////////////////////////







int main(){//simd衍生子实验，密集数组测试，规模100M
  
  data_capacity_ = 100000000;
  ALEX_DATA_NODE_KEY_AT = new int[data_capacity_];
  int unit = 9;//修改测试基数
  int test_key = unit, I = unit;//生成查找的目标键
  int start_pos = 9000;       //生成查找的起始位置    
  srand(time(NULL));
  for (int i = 0; i < 8; i++)
  {
    // //步长递增测试
    // test_key = unit*pow(10,i);
    // I = unit*pow(10,i);
    //随机数测试
    test_key = I = rand()%100000000;

    cout<<" start pos: "<<start_pos<<endl;
    for (int i = 0; i < data_capacity_; i++)
    {
      ALEX_DATA_NODE_KEY_AT[i] = i;
      if (i == I)
      {
        cout << " test_idex: " << i << endl;//应该查到的正确键的索引     
        cout << " test_key: " << ALEX_DATA_NODE_KEY_AT[i] << endl; //应该查到的正确键
      }
    }
    //simd_search
    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = std::chrono::high_resolution_clock::now(); //测试时间
    double per_search_time = 0.0;
    start_time = std::chrono::high_resolution_clock::now();
    int res_key = ALEX_DATA_NODE_KEY_AT[simd_search(start_pos, test_key)]; //实际查到的目标键值
    end_time = std::chrono::high_resolution_clock::now();
    per_search_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          end_time - start_time)
                          .count();
    cout << " --------------simd_res_key--------------: " << res_key << endl;
    cout << " simd_per_search_sec: " << per_search_time * 1e-9 << endl;

    //binary_search
    auto start_time2 = std::chrono::high_resolution_clock::now();
    auto end_time2 = std::chrono::high_resolution_clock::now(); //测试时间
    double per_search_time2 = 0.0;
    start_time2 = std::chrono::high_resolution_clock::now();
    int res_key2 = ALEX_DATA_NODE_KEY_AT[binary_search(start_pos, test_key)]; //实际查到的目标键值
    end_time2 = std::chrono::high_resolution_clock::now();
    per_search_time2 = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          end_time2 - start_time2)
                          .count();
    cout << " --------------bianry_res_key--------------: " << res_key2 << endl;
    cout << " binary_per_search_sec: " << per_search_time2 * 1e-9 << endl;

    //linear_search
    auto start_time3 = std::chrono::high_resolution_clock::now();
    auto end_time3 = std::chrono::high_resolution_clock::now(); //测试时间
    double per_search_time3 = 0.0;
    start_time3 = std::chrono::high_resolution_clock::now();
    int res_key3 = ALEX_DATA_NODE_KEY_AT[linear_search(start_pos, test_key)]; //实际查到的目标键值
    end_time3 = std::chrono::high_resolution_clock::now();
    per_search_time3 = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          end_time3 - start_time3)
                          .count();
    cout << " --------------linear_res_key--------------: " << res_key3 << endl;
    cout << " linear_per_search_sec: " << per_search_time3 * 1e-9 << endl;

    //binary_simd_search
    auto start_time4 = std::chrono::high_resolution_clock::now();
    auto end_time4 = std::chrono::high_resolution_clock::now(); //测试时间
    double per_search_time4 = 0.0;
    start_time4 = std::chrono::high_resolution_clock::now();
    int res_key4 = ALEX_DATA_NODE_KEY_AT[binary_simd_search(start_pos, test_key)]; //实际查到的目标键值
    end_time4 = std::chrono::high_resolution_clock::now();
    per_search_time4 = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          end_time4 - start_time4)
                          .count();
    cout << " --------------binary_simd_res_key--------------: " << res_key4 << endl;
    cout << " binary_simd_per_search_sec: " << per_search_time4 * 1e-9 << endl;

    //expo_bin_simd_search
    auto start_time5 = std::chrono::high_resolution_clock::now();
    auto end_time5 = std::chrono::high_resolution_clock::now(); //测试时间
    double per_search_time5 = 0.0;
    start_time5 = std::chrono::high_resolution_clock::now();
    int res_key5 = ALEX_DATA_NODE_KEY_AT[exponential_search_upper_bound_simd(start_pos, test_key)]; //实际查到的目标键值
    end_time5 = std::chrono::high_resolution_clock::now();
    per_search_time5 = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          end_time5 - start_time5)
                          .count();
    cout << " --------------expo_bin_simd_res_key--------------: " << res_key5 << endl;
    cout << " expo_bin_simd_per_search_sec: " << per_search_time5 * 1e-9 << endl;

    //expo_bin_nonsimd_search
    auto start_time6 = std::chrono::high_resolution_clock::now();
    auto end_time6 = std::chrono::high_resolution_clock::now(); //测试时间
    double per_search_time6 = 0.0;
    start_time6 = std::chrono::high_resolution_clock::now();
    int res_key6 = ALEX_DATA_NODE_KEY_AT[exponential_search_upper_bound(start_pos, test_key)]; //实际查到的目标键值
    end_time6 = std::chrono::high_resolution_clock::now();
    per_search_time6 = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          end_time6 - start_time6)
                          .count();
    cout << " --------------expo_bin_nonsimd_res_key--------------: " << res_key6 << endl;
    cout << " expo_bin_nonsimd_per_search_sec: " << per_search_time6 * 1e-9 << endl;

  }
  delete []ALEX_DATA_NODE_KEY_AT;
  return 0;
  
}
