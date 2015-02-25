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
	stockA -> value = 0;
	printStock(stockA);

	shmB = allocateSharedMemory(sizeof(struct stock));
	stockB = mapSharedMemory(shmB);
	stockB -> name = 'B';
	stockB -> value = 0;
	printStock(stockB);

	shmC = allocateSharedMemory(sizeof(struct stock));
	stockC = mapSharedMemory(shmC);
	stockC -> name = 'C';
	stockC -> value = 0;
	printStock(stockC);

	shmD = allocateSharedMemory(sizeof(struct stock));
	stockD = mapSharedMemory(shmD);
	stockD -> name = 'D';
	stockD -> value = 0;
	printStock(stockD);

	shmE = allocateSharedMemory(sizeof(struct stock));
	stockE = mapSharedMemory(shmE);
	stockE -> name = 'E';
	stockE -> value = 0;
	printStock(stockE);

	void catch(int);
	signal(SIGCHLD, catch);  // Detects child termination
	
	createWriters();
	createReaders();
	
	while(numChildrenAlive != 0)
	{
		printf("# of Children Alive: %d\n", numChildrenAlive);
		printf("Parent: Sleeping for 5 second...\n");
		sleep(5);
	}

	return 0;
}

void printStock(struct stock *s)
{
	char name = s -> name;
	double value = s -> value;
	printf("Stock %c: %lf\n", name, value);
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

createWriters()
{
	int i, pid;

	for (i = 1; i <= 4; i++)
	{
		if ((pid = fork()) != 0)
		{
			printf("Created reader: P%d -> %d\n", numChildrenAlive, pid);
			/* parent process pid != 0 */
			/* wait for child to terminate */
			numChildrenAlive++;
		}
		else if (pid == -1)
		{
			perror("Could not fork a writer process.\n");
		}
		else
		{
			/* child process pid = 0 */
			sleep(8);
			printf("Exited %d\n", getpid());
			exit(1);

		}
	}
}

createReaders()
{
	int i, pid;

	for (i = 1; i <= 3; i++)
	{
		if ((pid = fork()) != 0)
		{
			printf("Created reader: P%d -> %d\n", numChildrenAlive, pid);
			/* parent process pid != 0 */
			/* wait for child to terminate */
			numChildrenAlive++;
		}
		else if (pid == -1)
		{
			perror("Could not fork a reader process.\n");
		}
		else
		{
			/* child process pid = 0 */
			sleep(10);
			printf("Exited %d\n", getpid());
			exit(1);
		}
	}
}

void catch(int snum) {
	int pid;
	int status;

	pid = wait(&status);
	printf("Parent: Child Process pid=%d (%d).\n", pid, WEXITSTATUS(status));
	
	numChildrenAlive--;
	
	signal(SIGCHLD, catch);
}