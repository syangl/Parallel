#include<iostream>
#include<windows.h>
using namespace std;
int main()
{
    int n = 5, step = 20;
    long count = 0;

    long long head, tail, freq;

    for (n = 5; n <= 20000; n += step)
    {
        int* sum = new int[n];
        int* a = new int[n];
        for (int i = 0; i < n; i++)
            a[i] = i;
        int** m = new int* [n];
        for (int i = 0; i < n; i++)
            m[i] = new int[n];
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                m[i][j] = i + j;

        QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
        QueryPerformanceCounter((LARGE_INTEGER*)&head);
        QueryPerformanceCounter((LARGE_INTEGER*)&tail);
        while ((tail - head) / freq < 1)
        {
            count++;
            for (int i = 0; i < n; i++)
                sum[i] = 0;
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n; j++)
                    sum[j] += m[i][j] * a[i];
            QueryPerformanceCounter((LARGE_INTEGER*)&tail);
        }

        cout << "n = " << n << "  ms:" << (tail - head) * 1000.0 / freq / count << endl;


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
