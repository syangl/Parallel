#include<iostream>
#include<windows.h>
using namespace std;
int main()
{
    int n = 0, step = 2;
    long count = 0;

    long long head, tail, freq;

    for (n = 2; n <= 1500000; n += step)
    {
        int sum1 = 0, sum2 = 0, sum = 0;
        int* a = new int[n];
        for (int i = 0; i < n; i++)
            a[i] = i;

        QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
        QueryPerformanceCounter((LARGE_INTEGER*)&head);
        QueryPerformanceCounter((LARGE_INTEGER*)&tail);
        while ((tail - head) / freq < 1)
        {
            count++;
            for (int i = 0; i < n; i += 2)
            {
                sum1 += a[i];
                sum2 += a[i + 1];
            }
            sum = sum1 + sum2;
            QueryPerformanceCounter((LARGE_INTEGER*)&tail);
        }

        cout << "n = " << n << "  ms:" << (tail - head) * 1000.0 / freq / count << endl;


        step = n;

        count = 0;

    }

    return 0;
}

