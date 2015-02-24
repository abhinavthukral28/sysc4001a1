#include "functions.h"
#include <stdio.h>

struct stock
{
	char name;
	int value;
};

int int main(int argc, char const *argv[])
{
	struct stock *s;
	int shm = allocateSharedMemory(struct stock);
	s = mapSharedMemory(shm);
	return 0;
}