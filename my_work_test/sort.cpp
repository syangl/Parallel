#include<stdlib.h>
#include<ctime>
#include<chrono>
#include <iostream>
#include <algorithm>
using namespace std;

int main(int argc, char* argv[])
{
    int size = 10000000;   //数据大小
    cout<< "size="<<size<<endl;
    int* num = (int*)malloc(sizeof(int) * size);

    srand(time(NULL) + rand());   //生成随机数组
    for (int i = 0; i < size; i++)
        num[i] = rand();
    auto start_time2 = std::chrono::high_resolution_clock::now();
    sort(num, num+size);   //并行快排
    auto end_time2 = std::chrono::high_resolution_clock::now(); //测试时间

    double time2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time2 - start_time2).count();
    cout << "sort time: " << time2 * 1e-9 <<"s"<< endl;
    for (int i = 0; i < 10 && i<size; i++)//输出前十个元素
        cout<<num[i]<<" "; 
    cout<<endl;
    return 0;
}
