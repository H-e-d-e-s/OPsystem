/***************************************************************************
 *
 * Sequential version of Matrix-Matrix multiplication
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 1024

static double a[SIZE][SIZE];
static double b[SIZE][SIZE];
static double c[SIZE][SIZE];

struct threadArgs {
	int id;
};

static void
init_matrix(void)
{
    int i, j;

    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++) {
	        /* Simple initialization, which enables us to easy check
	         * the correct answer. Each element in c will have the same
	         * value as SIZE after the matmul operation.
	         */
	        a[i][j] = 1.0;
	        b[i][j] = 1.0;
        }
}

void*
matmul_seq(void *id)
{
    unsigned long i  = (unsigned long) id;
    int j,k;
    for (j = 0; j < SIZE; j++) {
        c[i][j] = 0.0;
        for (k = 0; k < SIZE; k++)
            c[i][j] = c[i][j] + a[i][k] * b[k][j];
        }
}

static void
print_matrix(void)
{
    int i, j;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++)
	        printf(" %7.2f", c[i][j]);
	    printf("\n");
    }
}

int
main(int argc, char **argv)
{
    pthread_t *threads;
    threads = malloc( SIZE * sizeof(pthread_t));
    struct threadArgs* args; // argument buffer
    init_matrix();
    for (unsigned long id = 0; id < SIZE; id++){
        args = malloc(sizeof(struct threadArgs)); // args vector
        args->id= id;
        pthread_create(&(threads[id]),NULL, matmul_seq,(void*)id);
    }
    for (unsigned long id = 0; id < SIZE; id++){
        pthread_join(threads[id],NULL);
    }
    print_matrix();
}
