#include <stdio.h>
 
#include <stdlib.h>
 
#include <unistd.h>
 
#include <sys/types.h>
 
#include <sys/ipc.h>
 
#include <sys/shm.h>
 
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h> /* For O_* constants */
 
const char *semName1 = "my_sem1";
const char *semName2 = "my_sem2";
 
 
 
 
#define SHMSIZE 128
 
#define SHM_R 0400
 
#define SHM_W 0200
 
#define SIZE_OF_BUFFER 10
 
#define EXIT_LOOP 1
 
 
 
 
 
int main(int argc, char **argv)
 
{
 
   int buffer_lenght = 0, write_index = 0, read_index = 0;
 
 
 
   struct shm_struct {
 
       int buffer[SIZE_OF_BUFFER];
 
       unsigned empty;
 
   };
 
   volatile struct shm_struct *shmp = NULL;
 
   char *addr = NULL;
 
   pid_t pid = -1;
 
   int var1 = 0, var2 = 0, shmid = -1, status;
 
   struct shmid_ds *shm_buf;
   time_t t;
 
 
 
   /* allocate a chunk of shared memory */
 
   shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | SHM_R | SHM_W);
 
   shmp = (struct shm_struct *) shmat(shmid, addr, 0);
 
   shmp->empty = 0;
 
   sem_t *sem_id1 = sem_open(semName1, O_CREAT, O_RDWR, 10);
  sem_t *sem_id2 = sem_open(semName2, O_CREAT, O_RDWR, 0);
 
   if (sem_id1 == SEM_FAILED || sem_id2 == SEM_FAILED)
   {
     perror("sem failed\n");
     exit(1);
   }
 
   pid = fork();
 
   if (pid != 0) {
 
       /* here's the parent, acting as producer */
 
       while (var1 < 100) {
 
           /* write to shmem */
 
           var1++;
 
            sem_wait(sem_id1);/* busy wait until the buffer is empty */
 
           if (buffer_lenght == SIZE_OF_BUFFER){
 
               write_index = 0;
 
               buffer_lenght = 0;
 
           }
           double radms= (double)rand()/RAND_MAX*(2.0 - 0.1)+ 0.1;
           sleep(radms);
           //printf("Random sending %f\n", radms);
 
           printf("Sending %d\n", var1); fflush(stdout);
 
           shmp->buffer[write_index]= var1;
 
           shmp->empty = 1;
 
           write_index++;
 
           buffer_lenght++;
 
           sem_post(sem_id2);
 
 
       }
 
       shmdt(addr);
 
       shmctl(shmid, IPC_RMID, shm_buf);
 
       sem_close(sem_id1);
       sem_close(sem_id2);
       wait(&status);
       sem_unlink(semName1);
       sem_unlink(semName2);
 
   } else {
 
       /* here's the child, acting as consumer */
 
       while (var2 < 100) {
 
           /* read from shmem */
 
           if (read_index == SIZE_OF_BUFFER){
 
               read_index = 0;
 
           }
 
           if (buffer_lenght = 0)
 
           {
 
               continue;
 
           }
           sem_wait(sem_id2);
      /* busy wait until there is something */
 
           var2 = shmp->buffer[read_index];
 
           read_index++;
 
           shmp->empty = 0;
           srand((int)time(&t) % getpid());
           double radmr= (double)rand()/RAND_MAX*(2.0 - 0.1)+ 0.1;
           sleep(radmr);
           //printf("Random receiving %f\n", radmr);
 
           printf("Received %d\n", var2); fflush(stdout);
           sem_post(sem_id1);
 
       }
       sem_close(sem_id1);
       sem_close(sem_id2);
 
       shmdt(addr);
 
       shmctl(shmid, IPC_RMID, shm_buf);
 
   }
 
}
