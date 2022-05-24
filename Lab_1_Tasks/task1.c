#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    pid_t c1_pid, c2_pid;
    unsigned i;
    unsigned niterations = 10;

    (c1_pid = fork()) && (c2_pid = fork());
    if (c1_pid == 0) {
        for (i = 0; i < niterations; ++i)
            printf("A = %d, ", i);
            
     } else if (c2_pid == 0) {
        for (i = 0; i < niterations; ++i)
            printf("B = %d, ", i);
    }
    else{
        for (i = 0; i < niterations; ++i)
            printf("C = %d, ", i);
        printf("\n");
        printf("child one id is: %d\n", c1_pid);
        printf("child two id is: %d\n", c2_pid);
        printf("Perant ID is:  %d\n", getpid());
    }
    printf("\n");
    printf("1aaaaa\n");
}
