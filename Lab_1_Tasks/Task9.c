#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct threadArgs {
	int id;
	int numThreads;
	int squaredId;
};

void* child(void* params) {
	struct threadArgs *args = (struct threadArgs*) params;
	args->squaredId = pow(args->id,2); //Squaring the childID 
}

int main(int argc, char** argv){
	pthread_t* children; // dynamic array of child threads
	struct threadArgs* args; // argument buffer
	unsigned int numThreads = 0;
	// get desired # of threads
	if (argc > 1)
		numThreads = atoi(argv[1]);
	int square[numThreads];
	children = malloc(numThreads * sizeof(pthread_t)); // allocate array of handles
	for (unsigned int id = 0; id < numThreads; id++) {
		// create threads
		args = malloc(sizeof(struct threadArgs)); // args vector
		args->numThreads = numThreads;
		args->id= id;
		pthread_create(&(children[id]), // our handle for the child
			NULL, // attributes of the child
			child, // the function it should run
			(void*)args); // args to that function
	}
	printf("I am the parent (main) thread.\n");
	for (unsigned int id = 0; id < numThreads; id++) 
	{
		pthread_join(children[id], (void**)&args->squaredId); 
		square[id] = args->squaredId;

	} 
	for (unsigned int id = 0; id < numThreads; id++) 
	{
		printf("Thread id: %d and square: %d\n", id,square[id]);	
	}
	free(args);
	free(children); // deallocate array
	return 0;
}