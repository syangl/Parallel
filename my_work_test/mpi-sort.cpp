#include"mpi.h"
#include<time.h>
#include <stdlib.h>
#include<ctime>
#include<chrono>
#include <iostream>
#include <algorithm>
#include <math.h>
using namespace std;

int parti(int* data, int start, int end)   
{
    int temp = data[start];   
    while (start < end) {
        while (start < end && data[end] >= temp)end--;   
        data[start] = data[end];
        while (start < end && data[start] <= temp)start++;    
        data[end] = data[start];
    }
    data[start] = temp;  
    return start;
}

//串行快排
void quick_sort(int* data, int start, int end)  
{
    if (start < end) {   
        int r = parti(data, start, end);
        quick_sort(data, start, r - 1);
        quick_sort(data, r + 1, end);
    }
}

int log(int n)
{
    int i = 1, j = 2;
    while (j < n) {
        j *= 2;
        i++;
    }
    return i;
}

void paraquick_sort(int* data, int start, int end, int m, int id, int nowID, int N)
{
    int r = end;
    int length = -1; 
    int* t;
    MPI_Status status;
    if (m == 0) {   
        if (nowID == id) quick_sort(data, start, end);
        return;
    }
    if (nowID == id) {    //进程负责分发
        while (id + (int)pow(2.0,m - 1) > N && m > 0) m--; 
        if (id + (int)pow(2.0,m - 1) < N) {  
            r = parti(data, start, end);
            length = end - r;
            MPI_Send(&length, 1, MPI_INT, id + (int)pow(2.0,m - 1), nowID, MPI_COMM_WORLD);
            if (length > 0)   //将后部数据发送给id+2^(m-1)进程
                MPI_Send(data + r + 1, length, MPI_INT, id + (int)pow(2.0,m - 1), nowID, MPI_COMM_WORLD);
        }
    }
    if (nowID == id + (int)pow(2.0,m - 1)) {    //进程接收
        MPI_Recv(&length, 1, MPI_INT, id, id, MPI_COMM_WORLD, &status);
        if (length > 0) {   
            t = (int*)malloc(length * sizeof(int));
            MPI_Recv(t, length, MPI_INT, id, id, MPI_COMM_WORLD, &status);
        }
    }
    int j = r - 1 - start;
    MPI_Bcast(&j, 1, MPI_INT, id, MPI_COMM_WORLD);
    if (j > 0)    
        paraquick_sort(data, start, r - 1, m - 1, id, nowID, N);  
    j = length;
    MPI_Bcast(&j, 1, MPI_INT, id, MPI_COMM_WORLD);
    if (j > 0)    
        paraquick_sort(t, 0, length - 1, m - 1, id + (int)pow(2.0,m - 1), nowID, N);  
    if ((nowID == id + (int)pow(2.0,m - 1)) && (length > 0))     
        MPI_Send(t, length, MPI_INT, id, id + (int)pow(2.0,m - 1), MPI_COMM_WORLD);
    if ((nowID == id) && id + (int)pow(2.0,m - 1) < N && (length > 0))     
        MPI_Recv(data + r + 1, length, MPI_INT, id + (int)pow(2.0,m - 1), id + (int)pow(2.0,m - 1), MPI_COMM_WORLD, &status);
}

int main(int argc, char* argv[])
{
    int* data;
    int rank, size;
    int n = 100000000;   //数组长度
    cout<<"n="<<n<<endl;
    double start_time, end_time;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &size);  
    if (rank == 0) {   //0进程生成随机数组
        data = (int*)malloc(n * sizeof(int));
        srand(time(NULL) + rand());  
        for (int i = 0; i < n; i++)
            data[i] = (int)rand();   
    }
    int m = log(size);  //第一次分发给第2^(m-1)个进程
    start_time = MPI_Wtime();
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);  //广播
    paraquick_sort(data, 0, n - 1, m, 0, rank, size); 
    end_time = MPI_Wtime();

    if (rank == 0) {  
        for (int i = 0; i < n && i < 10; i++)  
            cout<<data[i]<<" ";
        cout<<endl;
        cout<<" 并行时间："<<end_time - start_time<<"s"<<endl;
    }
    MPI_Finalize();
}
