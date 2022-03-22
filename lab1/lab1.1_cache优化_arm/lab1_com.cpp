#include<iostream>
#include<sys/time.h>
#include<time.h>
using namespace std;

void col(int n, int*& sum, int**& m, int*& a)
{
	for (int i = 0; i < n; i++)
	{
		sum[i] = 0;
		for (int j = 0; j < n; j++)
		{
			sum[i]+=m[j][i]*a[j];
		}
	}
}

int main()
{
	int n = 5,step = 20;
	long count = 0;
	int stime = 0;
	struct timeval start;
	struct timeval end;

	for(n = 5; n <= 30000; n+=step)
	{
		int *sum = new int[n];
   	     	int *a = new int[n];
        	srand((unsigned)time(NULL));
        	for (int i = 0; i < n; i++)
            	a[i] =  rand()%5+1;
      		int **m = new int*[n];
        	for(int i = 0; i < n; i++)
                	m[i] = new int[n];
        	for(int i = 0; i < n; i++)
                	for(int j = 0; j < n; j++)
                        	m[i][j] = rand()%5+1;
	
	        while(stime<100000)
        	{
           	 count++;
           	 gettimeofday(&start,NULL);
           	 col(n,sum,m,a);
           	 gettimeofday(&end,NULL);
           	 stime += ((end.tv_sec*1000000.0+end.tv_usec) - (start.tv_sec*1000000.0+start.tv_usec));
       	 	}
	
	        cout << "n = " << n << "  ms:" <<stime/1000.0/count<<" count:"<<count<< endl;

		
		if(n==125)
			step = 75;
		if(n==350)
			step = 1000;
		if(n==3350)
			step = 2000;
		if(n==11350)
			step = 10000;
		stime = 0;
		count = 0;

	}
	
	return 0;
}
