#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)
{

    /*
    Pipe: one way communication between processes.
    Child process writes to the file descriptor at the write end of the pipe, and the parent process reads from the read descriptor
    
    */
    //printf("parent pid: %d", (int)getpid());
    int pipefd[2];

    if (pipe(pipefd) == -1)
    {
        printf("pipe error");
        return 1;
    }
    int id = fork();
    if (id == 0)
    {
        //code for the child process

        //closing the read end of the pipe
        close(pipefd[0]);
        int x;
        printf("enter number:");
        scanf("%d", &x);

        //write x to the write end of the pipe
        write(pipefd[1], &x, sizeof(int));
        close(pipefd[1]);
    }
    else
    {
        //parent process code

        //parent process does not need write end
        close(pipefd[1]);
        int y;
        read(pipefd[0], &y, sizeof(int));
        close(pipefd[0]);
        printf("number from child process: %d\n", y);
    }
    return 0;
}