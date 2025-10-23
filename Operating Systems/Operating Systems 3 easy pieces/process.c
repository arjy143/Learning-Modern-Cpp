#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    printf("process id: &d\n", (int)getpid());

    int rc = fork();

    if (rc < 0)
    {
        //fork failed
        //fprintf places a string to a specified stream, compared to printf which places on stdout
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0)
    {
        //successully created child process
        printf("child (pid: %d)\n", (int)getpid());
    }
    else
    {
        //path for parent process
        printf("parent of %d ( pid: %d)\n", rc, (int)getpid());
    }
    return 0;
}