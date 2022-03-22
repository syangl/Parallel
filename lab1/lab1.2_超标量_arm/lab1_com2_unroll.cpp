#include<iostream>
#include<sys/time.h>
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
    int stime = 0;
    struct timeval start, end;

    for (n = 2; n <= 1500000; n += step)
    {
        int sum = 0,sum1 = 0, sum2 = 0;
        int* a = new int[n];
         srand((unsigned)time(NULL));
        for (int i = 0; i < n; i++)
            a[i] = rand()%5+1;

       	while(stime<100000)
	{	
            count++;
	    gettimeofday(&start,NULL);
            unroll(n,a,sum1,sum2,sum);
	    gettimeofday(&end,NULL);
	    stime += ((end.tv_sec*1000000.0+end.tv_usec) - (start.tv_sec*1000000.0+start.tv_usec));
	}

        cout << "n = " << n << "  ms:" <<stime/1000.0/count<<" count:"<<count<<" sum:"<<sum<< endl;
        step = n;
        count = 0;
	stime = 0;
    }

    return 0;
}
