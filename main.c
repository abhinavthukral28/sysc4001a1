#include "functions.h"
#include <stdio.h>

struct stock
{
	char name;
	int value;
};



int main(int argc, char const *argv[])
{
	struct stock *s;
	int shm = allocateSharedMemory(sizeof(struct stock));
	s = mapSharedMemory(shm);
	return 0;
}