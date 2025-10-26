#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    printf("process id: %d\n", (int)getpid());

    /*
    File descriptor just refers to a table that each process has that maps an integer id to a kernel file object like stdout.
    0 - stdin
    1 - stdout
    2 - stderr
    later ones are for other fd's like if you open a file somewhere
    */

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
    Use arguments or variants of wait, like waitpid, to wait for a specific child to finish.
    Waiting inside process without any children will return -1.
    */

    /*
    Exec system call lets you run a different bit of code on a child process.
    Successful exec call does not return anything.
    Various types of exec call, like execvp which acts on a file and arguments.
    Exec completely wipes the memory space of the child process
    */

    /*
    Using fork, wait and exec together lets you pipe and redirect output in bash easily.
    If i did something like "ls > out.txt", stdout (file descriptor = 1) is redirected to out.txt, then exec(ls) is called,
    which inherits that file descriptor, letting it output directly to out.txt.

    */

    int num = 1;
    int rc = fork();
    int irrelevant_process = fork();

    printf("irrelevant process pid: %d\n", irrelevant_process);

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
        ++num;
        printf("pid %d: num after fork = %d\n", (int)getpid(), num);
        printf("child (pid: %d)\n", (int)getpid());

        //printf("wait result: %d\n", wait());
        char* myargs[3];
        myargs[0] = strdup("ls");
        myargs[1] = strdup("-l");
        myargs[2] = 0;
        execvp(myargs[0], myargs);
        printf("Something went wrong if this prints");

        }
    else
    {
        printf("pid %d: num before wait = %d\n", (int)getpid(), num);
        int rc_wait = waitpid(rc);
        printf("pid %d: num after wait = %d\n", (int)getpid(), num);
        printf("rc_wait: %d\n", rc_wait);
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