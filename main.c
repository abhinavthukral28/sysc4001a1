#include "functions.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

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

// Number of total children processes that are still alive
int numChildrenAlive = 0;

// Setup Semaphores Sets for Writer/Reader Processes
int idReaderSem;
short readerValues[] = { 1, 2, 1, 2, 1 };

// Setup for Log File
FILE *logFile;

int main(int argc, char const *argv[])
{
	cleanup();

	// Setup Shared Memory for Stocks
	shmA = allocateSharedMemory(sizeof(struct stock));
	stockA = mapSharedMemory(shmA);
	stockA -> name = 'A';
	stockA -> value = 0.50;
	printStock(stockA, 0);

	shmB = allocateSharedMemory(sizeof(struct stock));
	stockB = mapSharedMemory(shmB);
	stockB -> name = 'B';
	stockB -> value = 1.00;
	printStock(stockB, 0);

	shmC = allocateSharedMemory(sizeof(struct stock));
	stockC = mapSharedMemory(shmC);
	stockC -> name = 'C';
	stockC -> value = 0.75;
	printStock(stockC, 0);

	shmD = allocateSharedMemory(sizeof(struct stock));
	stockD = mapSharedMemory(shmD);
	stockD -> name = 'D';
	stockD -> value = 2.25;
	printStock(stockD, 0);

	shmE = allocateSharedMemory(sizeof(struct stock));
	stockE = mapSharedMemory(shmE);
	stockE -> name = 'E';
	stockE -> value = 1.50;
	printStock(stockE, 0);

	// Creating Semaphore
	idReaderSem = createSemaphores(5, readerValues);

	// Initializing signal handler for detecting child termination
	void catch(int);
	signal(SIGCHLD, catch);
	
	// Spawning Writer/Reader Processes
	createWriters();
	createReaders();
	
	// Main process (parent) does not exit until all children are done
	while(numChildrenAlive != 0)
	{
		//printf("# of Children Alive: %d\n", numChildrenAlive);
		//printf("Parent: Sleeping for 1 second...\n");
		sleep(1);
	}

	// Destroy Semaphores
	fclose(logFile);
	deleteSemaphores(idReaderSem);

	return 0;
}

void createWriters()
{
	int i, j, pid;

	for (i = 0; i < 4; i++)
	{
		if ((pid = fork()) != 0) /* Main process, waits for children to terminate */
		{
			printf("Created writer: W%d -> %d\n", i, pid);
			//fprintf(logFile, "Created writer: P%d -> %d\n", numChildrenAlive, pid);
			numChildrenAlive++;
		}
		else if (pid == -1)
			perror("Could not fork a writer process.\n");
		else /* Writer process, pid = 0 */
		{	
			for (j = 0; j < 20; j++)
			{
				increaseStockPrice(stockB, i);
			}

			printf("\t\t\t\tExited Writer W%d -> %d\n", i, getpid());
			//fprintf(logFile, "\t\t\t\tExited Writer P%d -> %d\n", i, getpid());
			exit(1);
		}
	}
}

void createReaders()
{
	int i, j, pid;

	for (i = 0; i < 3; i++)
	{
		if ((pid = fork()) != 0) /* Main process, waits for children to terminate */
		{
			printf("Created reader: R%d -> %d\n", i, pid);
			//fprintf(logFile, "Created reader: P%d -> %d\n", numChildrenAlive, pid);
			numChildrenAlive++;
		}
		else if (pid == -1)
			perror("Could not fork a reader process.\n");
		else /* Reader process, pid = 0 */
		{
			for (j = 0; j < 25; j++)
			{
				readStock(stockB, i);
				sleep(1);
			}
			printf("\t\t\t\tExited Reader R%d -> %d\n", i, getpid());
			//fprintf(logFile, "\t\t\t\tExited Reader P%d -> %d\n", i, getpid());
			exit(1);
		}
	}
}

void printStock(struct stock *s, int proc)
{
	char name = s -> name;
	double value = s -> value;

	printf("%d: Stock %c: R%d: %0.2f\n", getTime(), name, proc, value);
	fprintf(logFile, "%d: Stock %c: R%d: %0.2f\n", getTime(), name, proc, value);
}

void readStock(struct stock *s, int proc)
{
	readLockSemaphore(idReaderSem, 1);
	//printf("%d: Reader -> %d:LOCKED\n", getTime(), getpid());
	//fprintf(logFile, "%d: Reader -> %d:LOCKED\n", getTime(), getpid());
	sleep(1);

	printStock(stockB, proc);
	
	readUnlockSemaphore(idReaderSem, 1);
	//printf("%d: Reader -> %d:UNLOCKED\n", getTime(), getpid());
	//fprintf(logFile, "%d: Reader -> %d:UNLOCKED\n", getTime(), getpid());
}

double randomPriceIncrement()
{
	/* Random Generator for price increment */
	srand(getpid());
	int rand_num;
	double d;

	rand_num = rand() % 5 + 1;
	d = (double) rand_num / (double) 100;

	return d;
}

void increaseStockPrice(struct stock *s, int proc)
{
	writeLockSemaphore(idReaderSem, 1, readerValues[1]);
	//printf("%d: Writer -> %d:LOCKED\n", getTime(), getpid());
	//fprintf(logFile, "%d: Writer -> %d:LOCKED\n", getTime(), getpid());
	double priceIncrement = randomPriceIncrement();	

	s -> value = (s -> value) + priceIncrement;

	char name = s -> name;
	double value = s -> value;

	writeUnlockSemaphore(idReaderSem, 1, readerValues[1]);
	//printf("%d: Writer -> %d:UNLOCKED\n", getTime(), getpid());
	//fprintf(logFile, "%d: Writer -> %d:UNLOCKED\n", getTime(), getpid());
	printf("%d: Stock %c: W%d: %0.2f\n", getTime(), name, proc, value);
	fprintf(logFile, "%d: Stock %c: W%d: %0.2f\n", getTime(), name, proc, value);
}

time_t getTime()
{
	time_t t;
	t = time(NULL);

	return t;
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

	// Logging Clean-Up
	remove("nolockingtraces2.log");
	logFile = fopen("nolockingtraces2.log", "a");
}

void catch(int snum)
{
	int pid;
	int status;

	pid = wait(&status);
	//printf("Parent: Child Process pid=%d (%d).\n", pid, WEXITSTATUS(status));
	
	numChildrenAlive--;
	
	signal(SIGCHLD, catch);
}