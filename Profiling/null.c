#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    int* ptr = malloc(sizeof(int)*100);
    
    //ptr[100] = 0;
    //printf("%d", a);
    free(ptr+10);
    int a = ptr[0];
    return 0;
}