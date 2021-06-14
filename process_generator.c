#include "headers.h"

void clearResources(int);

processDS *readProcesses(char *, int *);
bool sendReadyProcs(processDS *procs, int numProcs, int QKey);

processDS *procsList = NULL;
sem_t *msgQSemId;
pid_t schedId;
int msgQReady;

int main(int argc, char *argv[])
{
    signal(SIGUSR2, clearResources);
    int numProcesses;
    int shedulingType;

    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.

    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    printf("Please Enter Scheduling Algorithm:\n(1) FCFS (First Comes First Served\n(2) SJF (Shortest Job First)\n");
    printf("Your Choice: ");
    scanf("%d", &shedulingType);

    // Create Message Queue & its Semaphore

    // 3. Initiate and create the scheduler and clock processes.

    printf("generator.c: Creating Clock...\n");
    if (fork() == 0)
    {
        int clkStat = execl("clk.out", "clk.out", NULL);
        if (clkStat == -1)
        {
            perror("There was a problem Creating the Clock Process, generator.c");
            exit(-1);
        }
    }
    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function.
    int x = getClk();
    printf("generator.c: Strated at Time: %d\n", x);

    printf("generator.c: Creating Scheduler and passing Parameters...\n");
    schedId = fork();
    if (schedId == 0)
    {
        char type[2];
        sprintf(type, "%d", shedulingType);
        char *args[] = {"scheduler.out", type, NULL};
        int schedStat = execv(args[0], args);
        if (schedStat == -1)
        {
            perror("generator.c: Error in Executing Scheduler.out!!!, <generator.c>");
            exit(EXIT_FAILURE);
        }
    }

    else if (schedId == -1)
    {
        perror("generator.c: Error in Forking the Scheduler!!!, <generator.c>");
        exit(EXIT_FAILURE);
    }

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    procsList = NULL;
    procsList = (processDS *)readProcesses("processes.txt", &numProcesses);

    // Create Message Queue for the ReadyQueue
    printf("generator.c: Creating Message Queue...\n");
    msgQReady = msgget(MSGQKEY, 0666 | IPC_CREAT);
    if (msgQReady == -1)
    {
        perror("generator.c: Error in Creating Message Q, generator.c");
        exit(-1);
    }

    // use sem_post()-> Give Permission
    // sem_wait()-> wait for Permission
    printf("generator.c: Entering Generator Main Loop...\n");
    int curTime = -1;
    while (1)
    {
        if (getClk() != curTime)
        {
            curTime = getClk();
            // Sends process to Message queue if its turn comes
            bool isDone = sendReadyProcs(procsList, numProcesses, msgQReady);
            if (isDone)
                break;
        }

        // 6. Send the information to the scheduler at the appropriate time.
        // 7. Clear clock resources
    }
    int stat;
    waitpid(schedId, &stat, 0);
}

void clearResources(int signum)
{
    msgctl(msgQReady, IPC_RMID, (struct msqid_ds *)NULL);
    free(procsList);
    exit(EXIT_SUCCESS);
}

processDS *readProcesses(char *filename, int *numProcs)
{
    FILE *pFile;
    size_t lineLen;
    int id, arrival, run, priority;
    char *line = NULL;

    processDS *procsList = NULL;

    pFile = fopen("processes.txt", "r");
    if (pFile == NULL)
    {
        perror("Error Opening the Processes.txt, prcoess_generator");
        exit(-1);
    }

    // Read lines from the File
    while (getline(&line, &lineLen, pFile) != EOF)
    {
        if (line[0] == '#') // Skip lines with #
            continue;

        sscanf(line, "%d  %d  %d  %d", &id, &arrival, &run, &priority);
        (*numProcs)++;
        procsList = (processDS *)realloc(procsList, sizeof(processDS) * (*numProcs));
        procsList[*numProcs - 1].id = id;
        procsList[*numProcs - 1].arrivalTime = arrival;
        procsList[*numProcs - 1].runTime = run;
        procsList[*numProcs - 1].priority = priority;
    }

    fclose(pFile);
    return procsList;
}

bool sendReadyProcs(processDS *procs, int numProcs, int QKey)
{
    for (int i = 0; i < numProcs; i++)
    {
        if (procs[i].arrivalTime == getClk()) // if Process Arrived
        {
            // Send Process to Queue
            // 1- Prepare the message
            printf("generator.c: Sending Process to RQ: id %d arrival %d runTime %d priority %d\n", procs[i].id, procs[i].arrivalTime,
                   procs[i].runTime, procs[i].priority);
            struct msgbuff message;
            message.mtype = 1;
            message.proc = procs[i];
            int send_stat = msgsnd(QKey, &message, sizeof(message.proc), !IPC_NOWAIT);
            if (send_stat == -1)
            {
                perror("Problem in Sending the process to Ready Queue");
                exit(EXIT_FAILURE); // ??? let's Try it
            }
            printf("generator.c: Sending Signal to Scheduler...\n");
            kill(schedId, SIGUSR1);

            if (i == numProcs - 1)
            {
                if (procs[i].arrivalTime <= getClk())
                {
                    return true;
                }
            }
        }
    }
    return false;

    //

    // // read server response from DownQueue
    // int rec_val = msgrcv(msgq_dn, &message, sizeof(message.mtext), client_id, !IPC_NOWAIT);
}