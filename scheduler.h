#define MAXPT 100

typedef struct PMemory
{
    int id, arrival, remTime, runTime, proirity, waitTime;
    bool isRunning;
} PStatMemory;
