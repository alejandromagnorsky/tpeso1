#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <semaphore.h>
#include "../include/communication.h"

#define SIZE sizeof(Message)

// Semaphores
key_t keyRead = 1000;
key_t keyWrite = 2000;
int semRead;
int semWrite;

// For the shm
int serverFd; 
int clientFd;

int clientquant;


void 
sigHandler(){
	destroyIPC();
	exit(1);
}


void 
destroyIPC(){
	semctl(semRead, 0, IPC_RMID);
	semctl(semWrite, 0, IPC_RMID);
	shm_unlink("/server");
	shm_unlink("/client");	
}


void 
openServer(void * t){
	int i;
	clientquant = (int)t+3; // MODIFICAR
		
	if( (semRead = semget(keyRead, clientquant, 0666 | IPC_CREAT)) == -1) // Clientquant must take values between 500 and 2000
		errorLog("semget");
	if( (semWrite = semget(keyWrite, clientquant, 0666 | IPC_CREAT)) == -1) // Clientquant must take values between 500 and 2000
		errorLog("semget");

	for(i = 0; i < clientquant; i++)
		semctl(semWrite, i, SETVAL, 1);


	if ( (serverFd = shm_open("/server", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(serverFd, SIZE);

	if ( (clientFd = shm_open("/client", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(clientFd, clientquant*SIZE);

	Message * mem;
	if ( !(mem = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, serverFd, 0)) )
			errorLog("mmap");
	memset(mem, 0, SIZE);
	if ( !(mem = mmap(NULL, clientquant*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, clientFd, 0)) )
			errorLog("mmap");
	memset(mem, 0, clientquant*SIZE);
	
}



void 
openClient(void * t){
	clientquant = (int)t+3; // MODIFICAR
	
	if( (semRead = semget(keyRead, clientquant, 0666 | IPC_CREAT)) == -1)
		errorLog("semget");
	if( (semWrite = semget(keyWrite, clientquant, 0666 | IPC_CREAT)) == -1)
		errorLog("semget");


	if ( (serverFd = shm_open("/server", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(serverFd, SIZE);

	if ( (clientFd = shm_open("/client", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(clientFd, clientquant*SIZE);

	Message * mem;
	if ( !(mem = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, serverFd, 0)) )
			errorLog("mmap");
	if ( !(mem = mmap(NULL, clientquant*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, clientFd, 0)) )
			errorLog("mmap");
}


void
closeIPC(){
	close(serverFd);
	close(clientFd);
}


Message * 
receiveMessage(NodeType from, int key){

	Message * mem;
	Message * out = NULL;

	int index;

	if(from == SERVER) {	// Ant case
		if ( !(mem = mmap(NULL, clientquant*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, clientFd, 0)) )
			errorLog("mmap");
		index = key;
	}
	else {	// Map case
		if ( !(mem = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, serverFd, 0)) )
			errorLog("mmap");
		index = 0;
	}

	mem += index;

	struct sembuf semOpRead; // Operation for the write semaphore
	semOpRead.sem_num = index; // Number of semaphore in the array
	semOpRead.sem_op = -1;	

	struct sembuf semOpWrite; // Operation for the write semaphore
	semOpWrite.sem_num = index; // Number of semaphore in the array
	semOpWrite.sem_op = 1;


	semop(semRead, &semOpRead, 1);

	out = createMessage(mem->keyFrom, mem->keyTo, mem->opCode,  mem->param, mem->pos, mem->trace);

	semop(semWrite, &semOpWrite, 1);


	return out;
}

int 
sendMessage(NodeType to, Message * msg){

	Message * mem;	
	int index;
	
	if(to == SERVER){
		if ( !(mem = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, serverFd, 0)) )
			errorLog("mmap");
		index = 0;
	}
	else {
		if ( !(mem = mmap(NULL, clientquant*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, clientFd, 0)) )
			errorLog("mmap");
		index = msg->keyTo;
	}
	
	mem += index;
	
	struct sembuf semOpWrite; // Operation for the write semaphore
	semOpWrite.sem_num = index; // Number of semaphore in the array
	semOpWrite.sem_op = -1;
	
	struct sembuf semOpRead; // Operation for the write semaphore
	semOpRead.sem_num = index; // Number of semaphore in the array
	semOpRead.sem_op = 1;

	semop(semWrite, &semOpWrite, 1);

	memcpy(mem, msg, SIZE);

	semop(semRead, &semOpRead, 1);

	//printf("Mensaje mandado a %d. Enviado con keyTo = %d\n", index, mem->keyTo);

	return 0;
}