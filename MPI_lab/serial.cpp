#include <iostream>
#include <cstdlib>
#include <time.h>
#include<chrono>
#include <iomanip>
using namespace std;
#define MAX 100000
float** A = new float*[MAX+1];

void initMatrix(int n)
{
    for(int i=0;i<=n;i++)
    {
        for(int j=0;j<i;j++)
            A[i][j]=0.0;
        A[i][ i]=1.0;
        for(int j=i+1;j<=n;j++)
            A[i][j]=rand()%50000;
    }
    for(int k=0;k<=n;k++)
        for(int i=k+1;i<=n;i++)
            for(int j=0;j<=n;j++)
                A[i][j]+=A[k][j];
}

void normal(int n){
    for(int k = 1; k <= n; k++)
    {
        for (int j = k+1; j <= n; j++)
        {//simd
            A[k][j] = A[k][j]/A[k][k];
        }
        A[k][k] = 1.0;
        
        for (int i = k + 1; i <= n; i++){
            for (int j = k + 1; j <= n; j++)
            {//simd
                A[i][j] = A[i][j] - A[i][k]*A[k][j];
            }
            A[i][k] = 0.0;
        }
    }
}

void printMatrix(int n)
{
     for(int i = 1; i <= n ; i++)
    {
        for(int j = 1; j <= n; j++)
        {
            cout<<fixed<<setprecision(1)<<A[i][j]<<" ";
        }
        cout<<endl;
    }
}

int main()
{
    srand(time(NULL));
    for(int i = 0; i < MAX+1; i++)
    {
        A[i] = new float[MAX+1];
    }
    int n = 5000;
    //init A
    initMatrix(n+1);
    
    //normal compute
    auto t_start_time = std::chrono::high_resolution_clock::now();

    normal(n);

    auto t_end_time = std::chrono::high_resolution_clock::now();
    double t_time =
       std::chrono::duration_cast<std::chrono::nanoseconds>(t_end_time - t_start_time).count();
    std::cout << "---time: " << t_time*1e-9 <<" s"<< std::endl;

    //result print
    //printMatrix(n);

    return 0;
}