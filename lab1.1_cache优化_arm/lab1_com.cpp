#include<iostream>
#include<sys/time.h>
using namespace std;
int main()
{
	int n = 5,step = 20;
	long count = 0;

	struct timeval start;
	struct timeval end;

	for(n = 5; n <= 30000; n+=step)
	{
		int *sum = new int[n];
   	     	int *a = new int[n];
        	for(int i = 0; i < n; i++)
               		a[i]=i;
      		int **m = new int*[n];
        	for(int i = 0; i < n; i++)
                	m[i] = new int[n];
        	for(int i = 0; i < n; i++)
                	for(int j = 0; j < n; j++)
                        	m[i][j] = i + j;
	
		gettimeofday(&start,NULL);
		gettimeofday(&end,NULL);	
		while(((end.tv_sec+end.tv_usec/1000000.0) - (start.tv_sec+start.tv_usec/1000000.0)) < 1)
		{
			count++;
			for(int i = 0; i < n; i++)
			{		
				sum[i] = 0;
				for(int j = 0; j < n; j++)
					sum[i] += m[j][i]*a[j];
			}
			gettimeofday(&end,NULL);
		}
              	
                cout<<"n = "<<n<<"  ms:"<<(((end.tv_sec+end.tv_usec/1000000.0) - (start.tv_sec+start.tv_usec/1000000.0))/count)*1000<<endl;	

		
		if(n==125)
			step = 75;
		if(n==350)
			step = 1000;
		if(n==3350)
			step = 2000;
		if(n==11350)
			step = 10000;
		
		count = 0;

	}
	
	return 0;
}
