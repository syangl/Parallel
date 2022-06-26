#include<omp.h>
#include<time.h>
#include<stdlib.h>
#include <iostream>
using namespace std;

int parti(int* values, int start, int end)   
{
    int temp = values[start];  
    while (start < end) {
        while (start < end && values[end] >= temp)end--;  
        values[start] = values[end];
        while (start < end && values[start] <= temp)start++;    
        values[end] = values[start];
    }
    values[start] = temp;  
    return start;
}
//并行排序
void quick_sort(int* values, int start, int end)  
{
    if (start < end) {
        int pos = parti(values, start, end);
        #pragma omp parallel sections
        {
            #pragma omp section  
            //该区域对前部分数据进行排序        
            quick_sort(values, start, pos - 1);
            #pragma omp section          
            //该区域对后部分数据进行排序
            quick_sort(values, pos + 1, end);
        }
    }
}

int main(int argc, char* argv[])
{
    //线程数
    int n = 4;   
    int size = 180000000;   
    int* num = (int*)malloc(sizeof(int) * size);

    double start_time = omp_get_wtime(); 
    srand(time(NULL) + rand());  
    for (int i = 0; i < size; i++)
        num[i] = rand();
    //设置线程数
    omp_set_num_threads(n);   
    quick_sort(num, 0, size - 1);   
    double end_time = omp_get_wtime();

    for (int i = 0; i < 10 && i<size; i++)
        cout<<num[i]<<" "; 
    cout<<endl;
    cout<<" 并行时间："<<end_time - start_time<<"s"<<endl;
    return 0;
}
