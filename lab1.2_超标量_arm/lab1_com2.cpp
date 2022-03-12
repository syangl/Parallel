#include<iostream>
#include<sys/time.h>
#include<time.h>
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
    int stime = 0;
    struct timeval start, end;

    for (n = 2; n <= 1500000; n += step)
    {
        int sum = 0, sum1 = 0;
        int* a = new int[n];
         srand((unsigned)time(NULL));
        for (int i = 0; i < n; i++)
            a[i] = rand()%5+1;

        while(stime<100000)
	{	
            count++;
	    gettimeofday(&start,NULL);
            com2(n,a,sum,sum1);
	    gettimeofday(&end,NULL);
	    stime += ((end.tv_sec*1000000.0+end.tv_usec) - (start.tv_sec*1000000.0+start.tv_usec));
	}

        cout << "n = " << n << "  ms:" <<stime/1000.0/count<<" count:"<<count<<" sum:"<<sum<< endl;
	stime = 0;
        step = n;

        count = 0;

    }

    return 0;
}

