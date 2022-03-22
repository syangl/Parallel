#include<iostream>
#include<windows.h>
#include<time.h>
using namespace std;

void recursion(int n,int m, int* a,int& sum)
{
    for (m = n; m > 1; m /= 2)
        for (int i = 0; i < m/2; i++)
            a[i] = a[i*2] + a[i*2+1];
    sum = a[0];
}

void init(int* a, int* b, int n)
{
        for (int i = 0; i < n; i++)
            a[i] = b[i];
}

int main()
{
    int n = 0, step = 2, m = 0;
    int sum = 0;
    float stime = 0;
    long count = 0;

    long long head, tail, freq;

    for (n = 2; n <= 1500000; n += step)
    {
        int * a = new int [n];
        srand((unsigned)time(NULL));
        for (int i = 0; i < n; i++)
            a[i] = rand()%5+1;

        int* b = new int[n];
        for (int i = 0; i < n; i++)
            b[i] = a[i];

        QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

        while(stime<0.1)
        {
            count++;

            QueryPerformanceCounter((LARGE_INTEGER*)&head);
            recursion(n,m,a,sum);
            QueryPerformanceCounter((LARGE_INTEGER*)&tail);
            stime += (tail-head)*1.0/freq;

            init(a,b,n);
        }

        cout << "n = " << n << "  ms:" <<stime*1000.0/count<<" count:"<<count<<" sum:"<<sum<< endl;

        step = n;
        stime = 0;
        count = 0;
    }

    return 0;
}

