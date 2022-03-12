#include<iostream>
#include<windows.h>
#include<time.h>
using namespace std;

void col(int n,int*& sum, int**& m, int*& a)
{
    for (int i = 0; i < n; i++)
            {
                sum[i] = 0;
                for (int j = 0; j < n; j++)
                    sum[i] += m[j][i] * a[j];
            }
}

int main()
{
    int n = 5, step = 20;
    long count = 0;
    float stime = 0;
    long long head, tail, freq;

    for (n = 5; n <= 20000; n += step)
    {
        int* sum = new int[n];
        int* a = new int[n];
        srand((unsigned)time(NULL));
        for (int i = 0; i < n; i++)
            a[i] =  rand()%5+1;
        int** m = new int* [n];
        for (int i = 0; i < n; i++)
            m[i] = new int[n];
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                m[i][j] = rand()%5+1;

        QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

        while(stime<0.1)
        {
            count++;

            QueryPerformanceCounter((LARGE_INTEGER*)&head);
            col(n,sum,m,a);
            QueryPerformanceCounter((LARGE_INTEGER*)&tail);
            stime += (tail-head)*1.0/freq;
        }

        cout << "n = " << n << "  ms:" <<stime*1000.0/count<<" count:"<<count<< endl;


        stime = 0;

        if (n == 125)
            step = 75;
        if (n == 350)
            step = 1000;
        if (n == 3350)
            step = 2000;
        if (n == 11350)
            step = 10000;

        count = 0;

    }

    return 0;
}
