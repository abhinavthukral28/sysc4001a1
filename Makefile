all: Makefile shm.c main.c semaphores.c functions.h
	gcc main.c shm.c semaphores.c -lpthread -o stock 