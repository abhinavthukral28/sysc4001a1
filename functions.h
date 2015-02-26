#include <stdio.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <syslog.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/timeb.h>
#include <pthread.h>

struct stock
{
	char name;
	double value;
	int semvalue;
};

struct threadParameters
{
	struct stock *stk;
	int proc;
};

int allocateSharedMemory(int n);
void* mapSharedMemory(int id);
int createSemaphores(int n, short* vals);
void deleteSemaphores(int id);
void readLockSemaphore(int id, int i);
void readUnlockSemaphore(int id, int i);
void writeLockSemaphore(int id, int i, int value);
void writeUnlockSemaphore(int id, int i, int value);

void printStock(struct stock *s);
void readStock(struct stock *s, int proc);
void increaseStockPrice(struct stock *s, int proc);
double randomPriceIncrement();

void createReaders();
void createWriters();

void *readerJobThread(void* s);
void *writerJobThread(void* s);

double getTime();
void cleanup();
void randomSleep();