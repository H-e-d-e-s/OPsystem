#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h>


sem_t room;
sem_t chopstick[5];


void * philosopher(void *);
void eat(int);
void put_chopsick(int phil);

int main()
{
	int i,a[5];
	pthread_t tid[5];
	sem_init(&room,0,4);
	for(i=0;i<5;i++)
		sem_init(&chopstick[i],0,1);
	for(i=0;i<5;i++){
		a[i]=i;
		pthread_create(&tid[i],NULL,philosopher,(void *)&a[i]);
	}
	for(i=0;i<5;i++)
		pthread_join(tid[i],NULL);
}
void * philosopher(void * num)
{
	int x = 1;
	while (x = 1)
	{
		int phil=*(int *)num;
		sem_wait(&room);
		printf("Philosopher %d has entered room\n",phil);
		printf("Philosopher %d is Thinking\n",phil);
		sleep(((rand() % (10 + 1 - 2)) + 2)); //Thinking 2s-10s
		printf("Philosopher %d takes left chopstick \n",phil)    ;
		sleep(rand()%3 +1); //Thinking 1s-3s
		printf("Philosopher %d takes right chopstick \n",phil);
		sem_wait(&chopstick[phil]);
		sem_wait(&chopstick[(phil+1)%5]);
		eat(phil);
    	printf("Philosopher %d drop the left chopstick and Right chopstick \n",phil);
		printf("Philosopher %d has finished eating\n",phil);
		sem_post(&chopstick[(phil+1)%5]);
		sem_post(&chopstick[phil]);
		sem_post(&room);
	}
}
void eat(int phil)
{
	printf("Philosopher %d is eating\n",phil);
    sleep(((rand() % (20 + 1 - 10)) + 10)); // Eating in 10s-20s
}
