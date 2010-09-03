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
#define CLIENTQUANT 9 // NECESITO RECIBIR LA CANTIDAD DE CLIENTS

// Semaphore
sem_t * sd;
// For the shm
int serverFd; 
int clientFd;

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
openIPC(){
	if ( !(sd = sem_open(semKey, O_RDWR|O_CREAT, 0666, 1)) )	// Create and initialize the semaphore if isn't exists
		errorLog("sem_open");
	
	if ( (serverFd = shm_open("/server", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(serverFd, CLIENTQUANT*SIZE);

	if ( (clientFd = shm_open("/client", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(clientFd, CLIENTQUANT*SIZE);

	Message * mem;
	if ( !(mem = mmap(NULL, CLIENTQUANT*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, clientFd, 0)) )
			errorLog("mmap");
	memset(mem, 0,  CLIENTQUANT*SIZE);
	if ( !(mem = mmap(NULL, CLIENTQUANT*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, serverFd, 0)) )
			errorLog("mmap");
	memset(mem, 0, CLIENTQUANT*SIZE);
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

	printf("Recibiendo de FD: %d. Key: %d\n", fd, key);
	if ( !(mem = mmap(NULL, CLIENTQUANT*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) )
		errorLog("mmap");
	
	if(from == SERVER){ // The ant has to retrieve the message from an specific position in /client determinated by his key
		mem = mem + SIZE*key;
		sem_wait(sd);
		// Block the condition in the while, the assigment of out and the readed flag
		while(mem->keyTo != key){  // While the msg is read
			sem_post(sd);
			sleep(1);
			sem_wait(sd);
		}
	} else { // The map has to search the /server to verify if any ant send a message
		Message * aux;
		aux = mem;
		printf("Mapa recibiendo\n");	
		
		sem_wait(sd);
		for(i = 0; mem->keyTo != key; i++){ // While the msg is read
			mem = aux + SIZE * (i%CLIENTQUANT);
			sem_post(sd);
			sleep(1);
			printf("Key: %d. Recibio mensaje? FD: %d. Index: %d. KeyFrom:%d KeyTo:%d \n", key,  fd, i%CLIENTQUANT, mem->keyFrom, mem->keyTo);
			sem_wait(sd);
		}
	}

	out = createMessage(mem->keyFrom, mem->keyTo, mem->opCode,  mem->param, mem->pos, mem->trace);
	//FRONTEND ONLY
	out->fromPos.x = mem->fromPos.x;
	out->fromPos.y = mem->fromPos.y;

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
	printf("Enviando a FD: %d. Key: %d\n", fd, msg->keyFrom);
	
	if ( !(mem = mmap(NULL, CLIENTQUANT*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) )
		errorLog("mmap");

	if(to == SERVER)
		index = msg->keyFrom;
	else
		index = msg->keyTo;

	mem = mem + SIZE*index;
	
	// Block this zone of code
	sem_wait(sd);

	memcpy(mem, msg, SIZE);

	sem_post(sd);

	printf("Mensaje mandado a %d. Enviado con keyTo = %d\n", index, mem->keyTo);

	return 0;
}
