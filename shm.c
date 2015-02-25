#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <wait.h>
#include <time.h>
#include <stdlib.h>
#include <sys/shm.h>
#include "functions.h"


int allocateSharedMemory(int n)
{
	if(n > 0) 
	{
		return shmget(IPC_PRIVATE, n, IPC_CREAT | SHM_R | SHM_W);
	}
}

void* mapSharedMemory(int id)
{
	void* addr;
	if(id != 0)
	{
		addr = shmat(id, NULL, 0); 
		shmctl(id, IPC_RMID, NULL); 
		return addr;
	}
}

