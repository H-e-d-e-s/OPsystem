#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <time.h>
#define PERMS 0644
#define INT_MAX 100
#define INT_MIN 0
#define LIMIT 50

struct my_msgbuf {
   long mtype;
   int intger;
};

int main(void) {
   struct my_msgbuf buf;
   int msqid;
   key_t key;
   system("touch msgq.txt");

   if ((key = ftok("msgq.txt", 'B')) == -1) {
      perror("ftok");
      exit(1);
   }

   if ((msqid = msgget(key, PERMS | IPC_CREAT)) == -1) {
      perror("msgget");
      exit(1);
   }
   printf("message queue: ready to send messages.\n");
   buf.mtype = 1; /* we don't really care in this case */

   //The random generator
   srand(time(NULL));
   for(int counter = 0; counter < LIMIT; counter++)
   {

      // INT_MIN < msg_int < INT_MAx 
       buf.intger = (rand() % ((INT_MAX) -((INT_MIN) + 1))) + INT_MIN + 1;
       if (msgsnd(msqid, &buf, sizeof(buf.intger), 0) == -1) /* +1 for '\0' */
         perror("msgsnd");
       printf("Sending Number --> %d\n",buf.intger); 
       if(counter == LIMIT)
       {
           buf.intger = 0;
            if (msgsnd(msqid, &buf, sizeof(buf.intger), 0) == -1) /* +1 for '\0' */
                perror("msgsnd");

            if (msgctl(msqid, IPC_RMID, NULL) == -1) 
            {
                perror("msgctl");
                exit(1);
            }
       }
   }
   printf("message queue: done sending messages.\n");
   return 0;
}