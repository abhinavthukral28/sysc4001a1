#include "functions.h"
#include <stdio.h>

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

