#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


int allocateSharedMemory(int n)
{
	assert(n > 0); 
	return shmget(IPC_PRIVATE, n, IPC_CREAT | SHM_R | SHM_W);
}

void* mapSharedMemory(int id)
{
	void* addr;
	assert(id != 0);
	addr = shmat(id, NULL, 0); 
	shmctl(id, IPC_RMID, NULL); 
	return addr;
}



