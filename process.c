#include "headers.h"
#include <string.h>

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char *argv[])
{
    printf("process.c: Process Started...");
    initClk();
    // 1- get ID from args
    // if (agrc < 2)
    // {
    //     perror("Error in Process Arguments, Please pass the id in the arg list");
    //     exit(0);
    // }
    // int proc_id = atoi(argv[1]);

    // Get SHM Key
    // int shmRemId = shmget(proc_id, 4, 0666 | IPC_CREAT);
    printf("process.c: Creating SHM remTime...");
    int shmRemId = shmget(SHKEY, 4, 0666 | IPC_CREAT);
    if (shmRemId == -1)
    {
        perror("Error Gettign the Shared Memory ID... in process.c");
        exit(-1);
    }

    // 2- get sharedMem Address
    printf("process.c: Attaching to SHM remTime...");
    int *shmRemAdd = (int *)shmat(shmRemId, (void *)0, 0);
    if (*shmRemAdd == -1)
    {
        perror("process.c: Error Assigning space to Process, address Error, Process.c");
        exit(-1);
    }

    // Get initial remaining time
    remainingtime = __INT_MAX__;

    // 4- Get sem -> ????
    sem_t *semRemId = sem_open("SemaphoreName", O_CREAT, 0777, 0);
    if (semRemId == NULL)
    {
        perror("process.c: Error in Retreivign the Rem Semaphore, Process.c");
        exit(-1);
    }
    // 5- Create Semaphore -> ????
    int curTime;
    while (remainingtime > 0)
    {
        if (getClk() != curTime)
        {
            curTime = getClk();
            printf("process.c: checking SHM remTime...");
            sem_wait(semRemId);
            remainingtime = *shmRemAdd;
            if (remainingtime <= 0)
                break;
        }
    }
    printf("process.c: Process Finished...");

    destroyClk(false);
    // kill(getppid(), SIGUSR2);

    // kill self when remainingTime == 0
    return 0;
}
