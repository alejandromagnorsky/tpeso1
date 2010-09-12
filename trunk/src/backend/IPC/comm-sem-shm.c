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
#include "../../../include/transport.h"

// Semaphores
key_t keyRead = 1000;
key_t keyWrite = 2000;
int semRead;
int semWrite;

// For the shm
int serverFd; 
int clientFd;
char * memServer;
char * memClient;


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
openServer(void * t, int size){
	int i;
	clientquant = (int)t+3;
		
	if( (semRead = semget(keyRead, clientquant, 0666 | IPC_CREAT)) == -1) // Clientquant must take values between 500 and 2000
		errorLog("semget");
	if( (semWrite = semget(keyWrite, clientquant, 0666 | IPC_CREAT)) == -1) // Clientquant must take values between 500 and 2000
		errorLog("semget");

	for(i = 0; i < clientquant; i++){
		semctl(semWrite, i, SETVAL, 1);
		semctl(semRead, i, SETVAL, 0);
	}

	
	if ( (serverFd = shm_open("/server", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(serverFd, size);

	if ( (clientFd = shm_open("/client", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(clientFd, clientquant*size);


	if ( (memServer = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, serverFd, 0)) == (void *)-1)
			errorLog("mmap");
	memset(memServer, 0, size);
	if ( (memClient = mmap(NULL, clientquant*size, PROT_READ|PROT_WRITE, MAP_SHARED, clientFd, 0)) == (void *)-1)
			errorLog("mmap");
	memset(memClient, 0, clientquant*size);
	
}



void 
openClient(void * t, int size){
	clientquant = (int)t+3;
	
	if( (semRead = semget(keyRead, clientquant, 0666 | IPC_CREAT)) == -1)
		errorLog("semget");
	if( (semWrite = semget(keyWrite, clientquant, 0666 | IPC_CREAT)) == -1)
		errorLog("semget");


	if ( (serverFd = shm_open("/server", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(serverFd, size);

	if ( (clientFd = shm_open("/client", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(clientFd, clientquant*size);

	if ( (memServer = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, serverFd, 0)) == (void *)-1)
			errorLog("mmap");
	if ( (memClient = mmap(NULL, clientquant*size, PROT_READ|PROT_WRITE, MAP_SHARED, clientFd, 0)) == (void *)-1)
			errorLog("mmap");
}


void
closeServer(){
	close(serverFd);
	close(clientFd);

}

void
closeClient(){
	close(serverFd);
	close(clientFd);

}

 
// Key is client key
int 
receiveFromServer( int key, char * buf, int size ){
	char * mem;	
	mem = memClient + key*size;

	struct sembuf semOpRead; // Operation for the read semaphore
	semOpRead.sem_num = key; // Number of semaphore in the array
	semOpRead.sem_op = -1;	
	semOpRead.sem_flg = 0; // Set to wait

	struct sembuf semOpWrite; // Operation for the write semaphore
	semOpWrite.sem_num = key; // Number of semaphore in the array
	semOpWrite.sem_op = 1;
	semOpWrite.sem_flg = 0; // Set to wait

	semop(semRead, &semOpRead, 1);

	memcpy(buf, mem, size);
	
	semop(semWrite, &semOpWrite, 1);
	//printMessage(buf);
	return 0;
}


// Key is MAP_ID
int 
receiveFromClient( int key, char * buf, int size ){
	char * mem;
	mem = memServer;

	struct sembuf semOpRead; // Operation for the read semaphore
	semOpRead.sem_num = 0; // Number of semaphore in the array
	semOpRead.sem_op = -1;	
	semOpRead.sem_flg = 0; // Set to wait

	struct sembuf semOpWrite; // Operation for the write semaphore
	semOpWrite.sem_num = 0; // Number of semaphore in the array
	semOpWrite.sem_op = 1;
	semOpWrite.sem_flg = 0; // Set to wait

	semop(semRead, &semOpRead, 1);

	memcpy(buf, mem, size);
	
	semop(semWrite, &semOpWrite, 1);
	//printMessage(buf);
	return 0;
}


int 
sendToServer( int key, char * buf, int size ){
	char * mem;
	mem = memServer;

	struct sembuf semOpWrite; // Operation for the write semaphore
	semOpWrite.sem_num = 0; // Number of semaphore in the array
	semOpWrite.sem_op = -1;
	semOpWrite.sem_flg = 0; // Set to wait
	
	struct sembuf semOpRead; // Operation for the read semaphore
	semOpRead.sem_num = 0; // Number of semaphore in the array
	semOpRead.sem_op = 1;
	semOpRead.sem_flg = 0; // Set to wait

	semop(semWrite, &semOpWrite, 1);
	
	memcpy(mem, buf, size);
	
	semop(semRead, &semOpRead, 1);
	//printMessage(mem);
	return 0;
}


int 
sendToClient( int key, char * buf, int size ){
	char * mem;
	mem = memClient + key*size;
	
	struct sembuf semOpWrite; // Operation for the write semaphore
	semOpWrite.sem_num = key; // Number of semaphore in the array
	semOpWrite.sem_op = -1;
	semOpWrite.sem_flg = 0; // Set to wait
	
	struct sembuf semOpRead; // Operation for the read semaphore
	semOpRead.sem_num = key; // Number of semaphore in the array
	semOpRead.sem_op = 1;
	semOpRead.sem_flg = 0; // Set to wait

	semop(semWrite, &semOpWrite, 1);
	
	memcpy(mem, buf, size);
	
	semop(semRead, &semOpRead, 1);
	//printMessage(mem);
	return 0;
}



/*
Message * 
receiveMessage(NodeType from, int key){

	Message * mem;
	Message * out = malloc(SIZE);
	int index;

	if(from == SERVER) {	// Ant case
		mem = memClient;
		index = key;
	}
	else {	// Map case
		mem = memServer;
		index = 0;
	}

	mem += index;

	struct sembuf semOpRead; // Operation for the read semaphore
	semOpRead.sem_num = index; // Number of semaphore in the array
	semOpRead.sem_op = -1;	
	semOpRead.sem_flg = 0; // Set to wait

	struct sembuf semOpWrite; // Operation for the write semaphore
	semOpWrite.sem_num = index; // Number of semaphore in the array
	semOpWrite.sem_op = 1;
	semOpWrite.sem_flg = 0; // Set to wait

	semop(semRead, &semOpRead, 1);

	memcpy(out,mem,SIZE);
	
	semop(semWrite, &semOpWrite, 1);
	//printMessage(out);

	return out;
}



int 
sendMessage(NodeType to, Message * msg){

	Message * mem;	
	int index;
	
	if(to == SERVER){
		mem = memServer;
		index = 0;
	} else {
		mem = memClient;
		index = msg->keyTo;
	}

	mem += index;

	struct sembuf semOpWrite; // Operation for the write semaphore
	semOpWrite.sem_num = index; // Number of semaphore in the array
	semOpWrite.sem_op = -1;
	semOpWrite.sem_flg = 0; // Set to wait
	
	struct sembuf semOpRead; // Operation for the read semaphore
	semOpRead.sem_num = index; // Number of semaphore in the array
	semOpRead.sem_op = 1;
	semOpRead.sem_flg = 0; // Set to wait

	semop(semWrite, &semOpWrite, 1);
	
	memcpy(mem, msg, SIZE);
	
	semop(semRead, &semOpRead, 1);
	//printMessage(mem);
	

	return 0;
}*/

