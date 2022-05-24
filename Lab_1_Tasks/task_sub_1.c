#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(){
    unsigned i;
    unsigned niterations = 10;

    for (i = 0; i < niterations; ++i)
            printf("C = %d, ", i);
}