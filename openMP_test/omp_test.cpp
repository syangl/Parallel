#include <iostream>
#include <omp.h>

using namespace std;


int main(){
    int arr[20];int count = 0;int sum = 50; 

    #pragma omp parallel for num_threads(4)
        for(int i = 30; i < sum; i++){
            #pragma omp critical
                {    
                    arr[i-30] = 1;
                    cout<<"thread"<<omp_get_thread_num()<<"--i--"<<i<<endl<<endl;
                }
        
        }
   
    return 0;
}