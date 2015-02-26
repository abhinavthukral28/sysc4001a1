#include "functions.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

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


int main(int argc, char const *argv[])
{
	cleanup();

	// Setup Shared Memory for Stocks
	shmA = allocateSharedMemory(sizeof(struct stock));
	stockA = mapSharedMemory(shmA);
	stockA -> name = 'A';
	stockA -> value = 0.50;
	printStock(stockA);

	shmB = allocateSharedMemory(sizeof(struct stock));
	stockB = mapSharedMemory(shmB);
	stockB -> name = 'B';
	stockB -> value = 1.00;
	printStock(stockB);

	shmC = allocateSharedMemory(sizeof(struct stock));
	stockC = mapSharedMemory(shmC);
	stockC -> name = 'C';
	stockC -> value = 0.75;
	printStock(stockC);

	shmD = allocateSharedMemory(sizeof(struct stock));
	stockD = mapSharedMemory(shmD);
	stockD -> name = 'D';
	stockD -> value = 2.25;
	printStock(stockD);

	shmE = allocateSharedMemory(sizeof(struct stock));
	stockE = mapSharedMemory(shmE);
	stockE -> name = 'E';
	stockE -> value = 1.50;
	printStock(stockE);

	// Initializing signal handler for detecting child termination
	void catch(int);
	signal(SIGCHLD, catch);
	
	// Spawning writer and reader processes
	createWriters();
	createReaders();
	
	// Main process (parent) does not exit until all children are done
	while(numChildrenAlive != 0)
	{
		//printf("# of Children Alive: %d\n", numChildrenAlive);
		//printf("Parent: Sleeping for 5 second...\n");
		sleep(5);
	}

	return 0;
}

void printStock(struct stock *s)
{
	char name = s -> name;
	double value = s -> value;
	printf("Stock %c: %0.2f\n", name, value);
}

double randomPriceIncrement()
{
	/* Random Generator for price increment */
	srand(getpid());
	int rand_num;
	double d;

	rand_num = rand() % 5 + 1;
	d = (double) rand_num / (double) 100;
	printf("%0.2f\n", d);

	return d;
}

void increaseStockPrice(struct stock *s, double v)
{
	s -> value = (s -> value) + v;

	char name = s -> name;
	double value = s -> value;
	printf("Stock %c: %0.2f\n", name, value);
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

void createWriters()
{
	int i, pid;

	for (i = 0; i < 4; i++)
	{
		if ((pid = fork()) != 0) /* Main process, waits for children to terminate */
		{
			printf("Created writer: P%d -> %d\n", numChildrenAlive, pid);
			numChildrenAlive++;
		}
		else if (pid == -1)
			perror("Could not fork a writer process.\n");
		else /* Writer process, pid = 0 */
		{	
			double priceIncrement = randomPriceIncrement();
			increaseStockPrice(stockE, priceIncrement);

			sleep(10);
			printf("Exited Writer P%d -> %d\n", numChildrenAlive, getpid());
			exit(1);

		}
		sleep(1);
	}
}

void createReaders()
{
	int i, pid;

	for (i = 0; i < 3; i++)
	{
		if ((pid = fork()) != 0) /* Main process, waits for children to terminate */
		{
			printf("Created reader: P%d -> %d\n", numChildrenAlive, pid);
			numChildrenAlive++;
		}
		else if (pid == -1)
			perror("Could not fork a reader process.\n");
		else /* Reader process, pid = 0 */
		{
			int j;

			for (j = 0; j < 1; j++)
			{
				sleep(10);
			}

			printf("Exited Reader P%d -> %d\n", numChildrenAlive, getpid());
			exit(1);
		}
	}
}

void catch(int snum)
{
	int pid;
	int status;

	pid = wait(&status);
	printf("Parent: Child Process pid=%d (%d).\n", pid, WEXITSTATUS(status));
	
	numChildrenAlive--;
	
	signal(SIGCHLD, catch);
}