#include<iostream>
#include<windows.h>
#include <time.h>
using namespace std;

void com2(int n, int* a, int& sum, int sum1)
{
	for (int i = 0; i < n; i ++)
        {
   		sum1 += a[i];
        }
	sum = sum1;
}

int main()
{
    int n = 0, step = 2;
    long count = 0;
    float stime = 0;
    long long head, tail, freq;

    for (n = 2; n <= 1500000; n += step)
    {
        int sum = 0, sum1=0;
        int* a = new int[n];
        srand((unsigned)time(NULL));
        for (int i = 0; i < n; i++)
            a[i] =  rand()%5+1;

        QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

        while(stime<0.1)
        {
            count++;

            QueryPerformanceCounter((LARGE_INTEGER*)&head);
           com2(n,a,sum,sum1);
            QueryPerformanceCounter((LARGE_INTEGER*)&tail);
            stime += (tail-head)*1.0/freq;
        }

        cout << "n = " << n << "  ms:" <<stime*1000.0/count<<" count:"<<count<<" sum:"<<sum<< endl;


        stime = 0;
        step = n;

        count = 0;

    }

    return 0;
}

