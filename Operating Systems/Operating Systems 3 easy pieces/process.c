#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    printf("process id: %d\n", (int)getpid());

    /*
    Fork leads to 2 processes running the same code, but with different address spaces.
    A new pcb is allocated for the child process, with its own metadata and everything
    Only copied on write.
    Child gets copies of parents file descriptors, so they can write to the same files.
    Fork returns the child pid to the parent, and 0 to the child. on failure it returns -1 to the parent.
    */

    /*
    Wait system call makes the parent wait until the child is finished executing, then returns the child process ID to the parent.
    Parent in blocked state until child is finished.
    Use arguments or variants of wait, like waitpid, to wait for a specific child to finish
    */

    /*
    Exec system call lets you run a different bit of code on a child process.
    Successful exec call does not return anything.
    Various types of exec call, like execvp which acts on a file and arguments.
    Exec completely wipes the memory space of the child process
    */

    /*
    Using fork, wait and exec together lets you pipe and redirect output in bash easily.
    */
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
        char* myargs[3];
        myargs[0] = strdup("ls");
        myargs[1] = strdup("-l");
        myargs[2] = 0;
        execvp(myargs[0], myargs);
        printf("Something went wrong if this prints");

        }
    else
    {
        int rc_wait = wait(0);
        printf("rc_wait: %d", rc_wait);
        //path for parent process
        printf("parent of %d ( pid: %d)\n", rc, (int)getpid());
    }
    return 0;
}

// process id: 9312
// child (pid: 9313)
// total 24
// -rw-rw-r-- 1 arjun arjun  1839 Oct 25 14:25 process.c
// -rwxrwxr-x 1 arjun arjun 16352 Oct 25 14:25 process.exe
// -rw-rw-r-- 1 arjun arjun   103 Oct 25 11:17 README.md
// rc_wait: 9313parent of 9313 ( pid: 9312)