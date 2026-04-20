#include <stdio.h>
#include <assert.h>
#include <pthread.h>


void* mythread(void* arg)
{
   printf("%s\n", (char*)arg); 
   return NULL;
}

int main(int argc, char** argv)
{
    pthread_t p1;
    pthread_t p2;

    int rc;

    printf("main: begin\n");

    //either of the 2 threads could complete first, depending on the scheduler.
    pthread_create(&p1, NULL, mythread, "A");
    pthread_create(&p2, NULL, mythread, "B");

    //join takes in a pointer to the return value or null if void
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    printf("main: end\n");

    return 0;
}