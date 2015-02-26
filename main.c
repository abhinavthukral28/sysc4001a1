#include "functions.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <pthread.h>

//Pointers to Stocks
struct stock *stockA;
struct stock *stockB;
struct stock *stockC;
struct stock *stockD;
struct stock *stockE;

// Pointers to memory IDs
int shmA;
int shmB;
int shmC;
int shmD;
int shmE;

// Total Number of Active Child Processes
int numChildrenAlive = 0;
struct stock *readerStockList[3][3];
struct stock *writerStockList[4][3];

// Semaphores Set
int idReaderSem;
int activeProcessSem;

short readerValues[] = { 1, 2, 1, 2, 1 };
short activeProcessValues[] = { 1 };

// Timing
struct timeb starttime;

int main(int argc, char const *argv[])
{
	cleanup();

	// Get Program Start Time
	ftime(&starttime);

	// Setup Shared Memory for Stocks
	shmA = allocateSharedMemory(sizeof(struct stock));
	stockA = mapSharedMemory(shmA);
	stockA -> name = 'A';
	stockA -> value = 0.50;
	stockA -> semvalue = 0;
	printStock(stockA);

	shmB = allocateSharedMemory(sizeof(struct stock));
	stockB = mapSharedMemory(shmB);
	stockB -> name = 'B';
	stockB -> value = 1.00;
	stockB -> semvalue = 1;
	printStock(stockB);

	shmC = allocateSharedMemory(sizeof(struct stock));
	stockC = mapSharedMemory(shmC);
	stockC -> name = 'C';
	stockC -> value = 0.75;
	stockC -> semvalue = 2;
	printStock(stockC);

	shmD = allocateSharedMemory(sizeof(struct stock));
	stockD = mapSharedMemory(shmD);
	stockD -> name = 'D';
	stockD -> value = 2.25;
	stockD -> semvalue = 3;
	printStock(stockD);

	shmE = allocateSharedMemory(sizeof(struct stock));
	stockE = mapSharedMemory(shmE);
	stockE -> name = 'E';
	stockE -> value = 1.50;
	stockE -> semvalue = 4;
	printStock(stockE);

	printf("\n=================================\n\t  SIMULUATION\n=================================\n\n");

	// Creating Semaphores
	idReaderSem = createSemaphores(5, readerValues);
	activeProcessSem = createSemaphores(1, activeProcessValues);

	// Initializing signal handler for detecting child termination
	struct sigaction action;
	memset(&action, '\0', sizeof(action));
	action.sa_handler = childProcessHandler;

	if (sigaction(SIGCHLD, &action, 0))
	{
		perror("sigaction");
		return 1;
	}
	
	// Spawning Writer/Reader Processes
	createWriters();
	createReaders();
	
	// Main procedss (parent) does not exit until all children are done
	while(numChildrenAlive != 0)
	{
		sleep(1);
	}

	printf("\n=================================\n\t\tEND\n=================================\n\n");

	// Destroy Semaphores
	deleteSemaphores(idReaderSem);
	deleteSemaphores(activeProcessSem);

	return 0;
}

void createWriters()
{
	struct stock *writerStockList[4][3] = {
		{ stockA, stockB },
		{ stockA, stockB, stockC },
		{ stockC, stockD, stockE },
		{ stockD, stockE } };

	int i, j, k, pid, retvalue;

	// Creates 4 Writer Processes
	for (i = 0; i < 4; i++)
	{
		if ((pid = fork()) != 0) /* Main process, waits for children to terminate */
		{
			printf("\t\t\t\tCreated writer: W%d -> %d\n", i, pid);
			numChildrenAlive++;
		}
		else if (pid == -1)
			perror("Could not fork a writer process.\n");
		else /* Writer process, pid = 0 */
		{	
			// Loop 10 times for Simulation Purpose
			for (j = 0; j < 10; j++)
			{
				sleep(10);
				pthread_t threads[3];

				// Update Stocks Accessible by Current Writer Process
				for (k = 0; k < 3; k ++)
				{
					if (writerStockList[i][k] != NULL)
					{
						struct threadParameters *tP = malloc(sizeof(struct threadParameters));
						tP -> stk = writerStockList[i][k];
						tP -> proc = i;

						// Create Job Thread to Perform Stock Update
						retvalue = pthread_create(&threads[k], NULL, writerJobThread, tP);

						if (retvalue)
							printf("ERROR: thread not created. Code -> %d\n", retvalue);

						pthread_join(threads[k], NULL);
					}
				}
				//randomSleep();
			}
			printf("\t\t\t\tExited Writer W%d -> %d\n", i, getpid());
			exit(1);
		}
	}
}

void createReaders()
{
	struct stock *readerStockList[3][3] = {
		{ stockA, stockB },
		{ stockB, stockC, stockD },
		{ stockD, stockE } };

	int i, j, k, pid, retvalue;

	// Creates 4 Reader Processes
	for (i = 0; i < 3; i++)
	{
		if ((pid = fork()) != 0) /* Main process, waits for children to terminate */
		{
			printf("\t\t\t\tCreated reader: R%d -> %d\n", i, pid);
			numChildrenAlive++;
		}
		else if (pid == -1)
			perror("Could not fork a reader process.\n");
		else /* Reader process, pid = 0 */
		{
			// Loop 10 times for Simulation Purpose
			for (j = 0; j < 10; j++)
			{
				sleep(10);
				pthread_t threads[3];

				// Read Stocks Accessible by Current Reader Process
				for (k = 0; k < 3; k ++)
				{
					if (readerStockList[i][k] != NULL)
					{
						struct threadParameters *tP = malloc(sizeof(struct threadParameters));
						tP -> stk = readerStockList[i][k];
						tP -> proc = i;
					
						// Create Job Thread to Perform Stock Read
						retvalue = pthread_create(&threads[k], NULL, readerJobThread, tP);

						if (retvalue)
							printf("ERROR: thread not created. Code -> %d\n", retvalue);

						pthread_join(threads[k], NULL);
					}
				}
				//randomSleep();
			}
			printf("\t\t\t\tExited Reader R%d -> %d\n", i, getpid());
			exit(1);
		}
	}
}

void *readerJobThread(void* s)
{
	struct threadParameters* tP = (struct threadParameters*) s;

	readStock((tP -> stk), (tP -> proc));

	pthread_exit(NULL);
}

void *writerJobThread(void* s)
{
	struct threadParameters* tP = (struct threadParameters*) s;

	increaseStockPrice((tP -> stk), (tP -> proc));

	pthread_exit(NULL);
}

void printStock(struct stock *s)
{
	char name = s -> name;
	double value = s -> value;

	printf("%.3f: Stock %c: %0.2f\n", getTime(), name, value);
}

void readStock(struct stock *s, int proc)
{
	int semvalue = s -> semvalue;

	readLockSemaphore(idReaderSem, semvalue);
	//printf("%.3f: Stock %c: R%d: LOCKED\n", getTime(), s -> name, proc);
	printf("%.3f: Stock %c: R%d: %0.2f\n", getTime(), s -> name, proc, s -> value);
	
	readUnlockSemaphore(idReaderSem, semvalue);
	//printf("%.3f: Stock %c: R%d: UNLOCKED\n", getTime(), s -> name, proc);
}

double randomPriceIncrement()
{
	/* Random Generator for price increment */
	srand(time(NULL));
	int rand_num;
	double d;

	rand_num = rand() % 5 + 1;
	d = (double) rand_num / (double) 100;

	return d;
}

void increaseStockPrice(struct stock *s, int proc)
{
	int semvalue = s -> semvalue;
	double priceIncrement = randomPriceIncrement();	

	writeLockSemaphore(idReaderSem, semvalue, readerValues[semvalue]);
	//printf("%.3f: Stock %c: W%d: LOCKED\n", getTime(), s->name, proc);
	s -> value = (s -> value) + priceIncrement;
	printf("%.3f: Stock %c: W%d: %0.2f\n", getTime(), (s -> name), proc, (s -> value));

	writeUnlockSemaphore(idReaderSem, semvalue, readerValues[semvalue]);
	//printf("%.3f: Stock %c: W%d: UNLOCKED\n", getTime(), s->name, proc);
}

double getTime()
{
	struct timeb curtime;
	ftime(&curtime);

	double t = (curtime.time - starttime.time) + (double)((curtime.millitm - starttime.millitm) / 1000.0);

	return t;
}

void randomSleep()
{
	srand(getpid());
	int rand_num = rand() % 3000000 + 1000000;
	usleep(rand_num);
}

void cleanup()
{
	shmdt(stockA);
	shmctl(shmA,IPC_RMID,0);

	shmdt(stockB);
	shmctl(shmB,IPC_RMID,0);

	shmdt(stockC);
	shmctl(shmC,IPC_RMID,0);

	shmdt(stockD);
	shmctl(shmD,IPC_RMID,0);

	shmdt(stockE);
	shmctl(shmE,IPC_RMID,0);
}

static void childProcessHandler (int sig)
{
	int pid;
	int status;

	pid = wait(&status);

	readLockSemaphore(activeProcessSem, 0);
	//printf("Parent: Child Process pid=%d (%d).\n", pid, WEXITSTATUS(status));
	numChildrenAlive--;

	readUnlockSemaphore(activeProcessSem, 0);
}