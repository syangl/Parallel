#include<iostream>
#include<windows.h>
#include<time.h>
using namespace std;

void unroll(int n, int* a, int sum1, int sum2, int& sum)
{
  	for (int i = 0; i < n; i += 2)
        {
                sum1 += a[i];
                sum2 += a[i+1];
        }
        sum = sum1+sum2;

}

int main()
{
    int n = 0, step = 2;
    long count = 0;
    float stime = 0;
    long long head, tail, freq;

    for (n = 2; n <= 1500000; n += step)
    {
        int sum1 = 0, sum2 = 0, sum = 0;
        int* a = new int[n];
        srand((unsigned)time(NULL));
        for (int i = 0; i < n; i++)
            a[i] = rand()%5+1;

        QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

        while(stime<0.1)
        {
            count++;

            QueryPerformanceCounter((LARGE_INTEGER*)&head);
            unroll(n,a,sum1,sum2,sum);
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

