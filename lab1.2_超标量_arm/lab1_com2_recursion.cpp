#include<iostream>
#include<sys/time.h>
using namespace std;
int main()
{
    int n = 0, step = 2, m = 0;
    long count = 0;

    struct timeval start, end;

    for (n = 2; n <= 1500000; n += step)
    {
        int* a = new int[n];
        for (int i = 0; i < n; i++)
            a[i] = i;

        gettimeofday(&start,NULL);
        gettimeofday(&end,NULL);
        while (((end.tv_sec+end.tv_usec/1000000.0) - (start.tv_sec+start.tv_usec/1000000.0))< 1)
        {
            count++;
            for (m = n; m > 1; m /= 2) 
	    	for (int i = 0; i < m / 2; i++)
			a[i] = a[i*2] + a[i*2+1];
            gettimeofday(&end,NULL);
        }

        cout << "n = " << n << "  ms:" << ((end.tv_sec+end.tv_usec/1000000.0) - (start.tv_sec+start.tv_usec/1000000.0))*1000.0/count << endl;
        step = n;
        count = 0;

    }

    return 0;
}
