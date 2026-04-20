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

    pthread_create(&p1, NULL, mythread, "A");
    pthread_create(&p2, NULL, mythread, "B");

    printf("main: end\n");

    return 0;
}