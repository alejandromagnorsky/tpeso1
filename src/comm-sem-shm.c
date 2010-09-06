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
#include <semaphore.h>
#include <errno.h>
#include "../include/communication.h"

#define SIZE sizeof(Message)
#define semKey "/mutex"


// Semaphore
sem_t * sd;
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
	sem_unlink(semKey);
	shm_unlink("/server");
	shm_unlink("/client");	
}


void 
openServer(void * t){
	clientquant = (int)t+4;
	if ( !(sd = sem_open(semKey, O_RDWR|O_CREAT, 0666, 1)) )	// Create and initialize the semaphore if isn't exists
		errorLog("sem_open");
	
	if ( (serverFd = shm_open("/server", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(serverFd, clientquant*SIZE);

	if ( (clientFd = shm_open("/client", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(clientFd, clientquant*SIZE);

	Message * mem;
	if ( !(mem = mmap(NULL, clientquant*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, clientFd, 0)) )
			errorLog("mmap");
	memset(mem, 0, clientquant*SIZE);
	if ( !(mem = mmap(NULL, clientquant*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, serverFd, 0)) )
			errorLog("mmap");
	memset(mem, 0, clientquant*SIZE);
}



void 
openClient(void * t){
	clientquant = (int)t+4;
	if ( !(sd = sem_open(semKey, O_RDWR|O_CREAT, 0666, 1)) )	// Create and initialize the semaphore if isn't exists
		errorLog("sem_open");
	
	if ( (serverFd = shm_open("/server", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(serverFd, clientquant*SIZE);

	if ( (clientFd = shm_open("/client", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(clientFd, clientquant*SIZE);

	Message * mem;
	if ( !(mem = mmap(NULL, clientquant*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, clientFd, 0)) )
			errorLog("mmap");
	if ( !(mem = mmap(NULL, clientquant*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, serverFd, 0)) )
			errorLog("mmap");
}


void
closeIPC(){
	sem_close(sd);
	close(serverFd);
	close(clientFd);
}


Message * 
receiveMessage(NodeType from, int key){

	Message * mem;
	Message * out = NULL;

	int fd, i;

	if(from == SERVER) // Ant case
		fd = clientFd;
	else				// Map case
		fd = serverFd;

	//printf("Recibiendo de FD: %d. Key: %d\n", fd, key);
	if ( !(mem = mmap(NULL, clientquant*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) )
		errorLog("mmap");
	
	if(from == SERVER){ // The ant has to retrieve the message from an specific position in /client determinated by his key
		mem += key;
		sem_wait(sd);
		// Block the condition in the while, the assigment of out and the readed flag
		while(mem->keyTo != key){  // While the msg is read
			sem_post(sd);
		//	sleep(1);
			sem_wait(sd);
		}
	} else { // The map has to search the /server to verify if any ant send a message
		Message * aux;
		aux = mem;
				
		sem_wait(sd);
		for(i = 0; mem->keyTo != key; i++){ // While the msg is read
			mem = aux + i%clientquant;
			sem_post(sd);
			//sleep(1);
			//printf("Key: %d. Recibio mensaje? FD: %d. Index: %d. KeyFrom:%d KeyTo:%d \n", key,  fd, i%clientquant, mem->keyFrom, mem->keyTo);
			sem_wait(sd);
		}
	}

	out = createMessage(mem->keyFrom, mem->keyTo, mem->opCode,  mem->param, mem->pos, mem->trace);

	mem->keyTo = 0; // Mark the message as readed
	
	sem_post(sd);

	return out;
}

int 
sendMessage(NodeType to, Message * msg){

	Message * mem;	
	int fd, index;
	
	if(to == SERVER)
		fd = serverFd;	
	else
		fd = clientFd;
//	printf("Key: %d enviando a FD: %d.\n", msg->keyFrom, fd);
	
	if ( !(mem = mmap(NULL, clientquant*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) )
		errorLog("mmap");

	if(to == SERVER)
		index = msg->keyFrom;
	else
		index = msg->keyTo;

	mem += index;
	
	while(mem->keyTo != 0) // While the msg is not read	
		printf("HOLAAAAAAAAAAAAAA\n");
		
	
	// Block this zone of code
	sem_wait(sd);	

	memcpy(mem, msg, SIZE);

	sem_post(sd);

	//printf("Mensaje mandado a %d. Enviado con keyTo = %d\n", index, mem->keyTo);

	return 0;
}
