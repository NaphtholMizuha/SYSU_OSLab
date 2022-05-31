#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("call fork\n");

    pid_t pid = fork();
    if (pid < 0)
    {
        printf("fork error\n");
        exit(-1);
    }

    printf("fork return\n");

    if (pid)
    {
        printf("father fork return: %d\n", pid);
    }
    else
    {
        printf("child fork return: %d\n", pid);
    }
}