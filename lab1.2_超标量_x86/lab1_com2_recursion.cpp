#include<iostream>
#include<windows.h>
using namespace std;
int main()
{
    int n = 0, step = 2, m = 0;
    long count = 0;

    long long head, tail, freq;

    for (n = 2; n <= 1500000; n += step)
    {
        int* a = new int[n];
        for (int i = 0; i < n; i++)
            a[i] = i;

        QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
        QueryPerformanceCounter((LARGE_INTEGER*)&head);
        QueryPerformanceCounter((LARGE_INTEGER*)&tail);
        while ((tail - head) / freq < 1)
        {
            count++;
            for (m = n; m > 1; m /= 2)
                for (int i = 0; i < m / 2; i++)
                    a[i] = a[i*2] + a[i*2+1];
            QueryPerformanceCounter((LARGE_INTEGER*)&tail);
        }

        cout << "n = " << n << "  ms:" << (tail - head) * 1000.0 / freq / count << endl;


        step = n;

        count = 0;

    }

    return 0;
}

