#include<iostream>
#include<sys/time.h>
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
    int n = 0, step = 2, m = 0,sum = 0;
    long count = 0;
    long stime = 0;

    struct timeval start, end, b1, b2;

    for (n = 2; n <= 1500000; n += step)
    {
        int* a = new int[n];
        srand((unsigned)time(NULL));
        for (int i = 0; i < n; i++)
            a[i] = rand()%5+1;

	int* b = new int[n];
        for (int i = 0; i < n; i++)
            b[i] = a[i];	

        
	while(stime<100000)
	{	
            count++;
	    gettimeofday(&start,NULL);
            recursion(n,m,a,sum);
	    gettimeofday(&end,NULL);
	    stime += ((end.tv_sec*1000000.0+end.tv_usec) - (start.tv_sec*1000000.0+start.tv_usec));
	    
	    init(a,b,n);
	}

        cout << "n = " << n << "  ms:" <<stime/1000.0/count<<" count:"<<count<<" sum:"<<sum<< endl;
        step = n;
        count = 0;
	stime = 0;
	delete[]b;
    }

    return 0;
}

