#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>


int chopstick[5]= {1,2,3,4,5};

void * philosopher(void *);
void eat(int);
void put_chopsick(int phil);

int main()
{
	int i,a[5];
	pthread_t tid[5];
	
	for(i=0;i<5;i++){
		a[i]=i;
		pthread_create(&tid[i],NULL,philosopher,(void *)&a[i]);
	}
	for(i=0;i<5;i++)
		pthread_join(tid[i],NULL);
}
void * philosopher(void * num)
{
	int phil=*(int *)num;
	printf("Philosopher %d has entered room\n",phil);
    printf("Philosopher %d is Thinking\n",phil);
    sleep(((rand() % (10 + 1 - 2)) + 2)); //Thinking 2s-10s
    printf("Philosopher %d takes left chopstick %d\n",phil, chopstick[phil])    ;
    sleep(rand()%3 +1); //Thinking 1s-3s
    printf("Philosopher %d takes right chopstick %d\n",phil, chopstick[(phil+1)%5]);
	eat(phil);
	printf("Philosopher %d has finished eating\n",phil);

}
void eat(int phil)
{
	printf("Philosopher %d is eating\n",phil);
    sleep(((rand() % (20 + 1 - 10)) + 10)); // Eating in 10s-20s
    put_chopsick(phil);
}
void put_chopsick(int phil){
    printf("Philosopher %d drop the left chopstick %d and Right chopstick %d\n",phil,chopstick[phil],chopstick[(phil+1)%5]);
}
