#include "headers.h"
#include "scheduler.h"

void addProcFromReady(int);
void applyFCFS();
void setNextInReady();
void createProcessTable(int);

processDS *ReadyProcs;
processDS *SortedProcs;
PStatMemory *p_stat_ready = NULL; // Ready Queue Process's PCB

int numReady = 0;
sem_t *semRemId;
int Qkey;

int *shmRemAdd;

bool isSortSJF = true;

int main(int argc, char *argv[])
{
    signal(SIGUSR1, addProcFromReady);
    printf("Sched.c: Scheduler Started...\n");
    initClk();

    //TODO: implement the scheduler.
    // 1- Crate Message Queue to get Ready procs
    Qkey = msgget(MSGQKEY, 0666 | IPC_CREAT);
    if (Qkey == -1)
    {
        perror("Sched.c: Error Creating the message Queue");
        exit(EXIT_FAILURE);
    }

    // 2- create SHM to access processes RemainingTime
    printf("Sched.c: Creating SHM for Remaining Time\n");
    int shmRemId = shmget(SHKEY, 4, 0666 | IPC_CREAT);
    if (shmRemId == -1)
    {
        perror("Sched.c: Error Gettign the Shared Memory ID...");
        exit(EXIT_FAILURE);
    }

    printf("Sched.c: Attaching SHM for Remaining Time\n");
    shmRemAdd = (int *)shmat(shmRemId, (void *)0, 0);
    if (*shmRemAdd == -1)
    {
        perror("Sched.c: Error Assigning space to Process, address Error");
        exit(EXIT_FAILURE);
    }

    // 3- Remaining Time SemaPhore
    printf("Sched.c: Creating RemTime SemaPhore...\n");
    sem_t *semRemId = sem_open("SemaphoreName", O_CREAT, 0777, 0);
    if (semRemId == NULL)
    {
        perror("Sched.c: Error in Retreivign the Rem Semaphore...");
        exit(-1);
    }

    int schedType = (argc > 1) ? atoi(argv[1]) : 0;
    printf("Sched.c: Scheduling Type: %d\n", schedType);
    int curTime = -1;

    // Create Process Table
    createProcessTable(MAXPT);

    while (1)
    {
        if (curTime != getClk())
        {
            curTime = getClk();
            if (numReady > 0)
            {
                switch (schedType)
                {
                case 0:          //FCFS
                    applyFCFS(); // works on the ready Queue
                    break;

                case 1: // SJF
                    // sortSJF();
                    // applySJF();
                    break;
                }
            }
        }
    }
    //TODO: upon termination release the clock resources.

    destroyClk(true);
}

void addProcFromReady(int signum)
{
    printf("sched.c: Received Signal from generator...\n");
    struct msgbuff message;
    printf("Sched.c: receiving processes from message Queue\n");
    int rec_stat = msgrcv(Qkey, &message, sizeof(message), 0, !IPC_NOWAIT);
    if (rec_stat == -1)
    {
        perror("Sched.c: error Receiving the process from message Queue");
        exit(EXIT_FAILURE);
    }
    // add process ro Ready Queue
    printf("Sched.c: Process %d arrived at %d with runTime %d and priority %d\n", message.proc.id,
           message.proc.arrivalTime, message.proc.runTime, message.proc.priority);
    printf("Adding Process to Processes List...\n");
    p_stat_ready[numReady].id = message.proc.id;
    p_stat_ready[numReady].arrival = message.proc.arrivalTime;
    p_stat_ready[numReady].runTime = message.proc.runTime;
    p_stat_ready[numReady].proirity = message.proc.priority;
    p_stat_ready[numReady].isRunning = false;
    p_stat_ready[numReady].remTime = message.proc.runTime;
    p_stat_ready[numReady].waitTime = 0;
    numReady++;
    signal(SIGUSR1, addProcFromReady);
}

void applyFCFS()
{
    if (p_stat_ready[0].isRunning == false)
        p_stat_ready[0].isRunning = true;
    // get p_stat_mem and decrease Remaining Time
    int remtime = p_stat_ready->remTime;

    remtime--;
    if (remtime <= 0)
    {
        // 1- write to shm remTime
        *shmRemAdd = remtime;
        sem_post(semRemId);
        // 2- delete from ReadyQ
        printf("At Time %d: ", getClk());
        printf("Deleting Process %d arrival %d priority %d\n", p_stat_ready->id,
               p_stat_ready->arrival, p_stat_ready->proirity);
        setNextInReady();
        return;
    }
    p_stat_memory->remTime = remtime;
    printf("Setting Process %d arrival %d remTime %d priority %d\n", p_stat_memory->id,
           p_stat_memory->arrival, p_stat_memory->remTime, p_stat_memory->proirity);
}

void setNextInReady()
{
}

void createProcessTable(int max)
{
    p_stat_ready = malloc(sizeof(PStatMemory) * max);
    numReady = 0;
}
// void sortSJF(){

// }

// void applySJF(){
//     if(numReady > 1){
//         for()
//     }
// }