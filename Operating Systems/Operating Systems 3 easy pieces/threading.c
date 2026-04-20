#include <stdio.h>
#include <assert.h>
#include <pthread.h>


void* mythread(void* arg)
{
    long long value = (long long)arg;
    printf("%lld\n", value); 
   return (void*)(value+1);
}

int main(int argc, char** argv)
{
    pthread_t p1;
    pthread_t p2;

    long long return_value;

    printf("main: begin\n");

    //either of the 2 threads could complete first, depending on the scheduler.
    pthread_create(&p1, NULL, mythread, (void*)100);

    //join takes in a pointer to the return value or null if void
    pthread_join(p1, (void**)return_value);

    pthread_create(&p2, NULL, mythread, (void*)return_value);

    pthread_join(p2, (void**)return_value);

    printf("main: end\n");

    return 0;
}