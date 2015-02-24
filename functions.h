#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int allocateSharedMemory(int n);
void* mapSharedMemory(int id);