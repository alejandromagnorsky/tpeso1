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
#define ANTQUANT 5

static char * semKey = "/mutex";
static sem_t * sd;
// For the shm
static int serverFd; 
static int clientFd;

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

void openIPC(){
	if ( !(sd = sem_open(semKey, O_RDWR|O_CREAT, 0666, 1)) )	// Create and initialize the semaphore if isn't exists
		errorLog("sem_open");
	
	if ( (serverFd = shm_open("/server", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(serverFd, ANTQUANT*SIZE);

	if ( (clientFd = shm_open("/client", O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(clientFd, ANTQUANT*SIZE);

	Message * memClt;
	Message * memSrv;
	if ( !(memClt = mmap(NULL, ANTQUANT*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, clientFd, 0)) )
			errorLog("mmap");
	memset(memClt, 0, ANTQUANT*SIZE);
	if ( !(memSrv = mmap(NULL, ANTQUANT*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, serverFd, 0)) )
			errorLog("mmap");
	memset(memSrv, 0, ANTQUANT*SIZE);
}


void
closeIPC(){
	sem_close(sd);
	close(serverFd);
	close(clientFd);
}


Message * 
receiveMessage(NodeType from){

	Message * mem;
	Message * out = NULL;
	Message * aux;
	int fd, i = 1;

	if(from == SERVER) // Ant case
		fd = clientFd;
	else				// Map case
		fd = serverFd;

	printf("FD: %d. PID: %d\n", fd, getpid());
	if ( !(mem = mmap(NULL, ANTQUANT*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) )
		errorLog("mmap");
	
	aux = mem;		
	printf("Recibiendo\n");

	//Block the condition in the while, the assigment of out and the readed flag
	sem_wait(sd);
	while( !( mem->keyTo == getpid() || (from == CLIENT && mem->keyTo == -1) ) ){ // While the msg is readed or is for another ant and it isn't from the server
		mem = aux+SIZE*(i%ANTQUANT);
		sem_post(sd);
		sleep(1);
		printf("PID: %d. Recibio mensaje? %d.KeyFrom:%d KeyTo:%d \n", getpid(), i%ANTQUANT, mem->keyFrom, mem->keyTo);
		sem_wait(sd);
		i++;
	}

	// A copy must be made, because mem is deallocated after this function
	out = createMessage(mem->keyFrom, mem->keyTo, mem->opCode,  mem->param, mem->pos, mem->trace);

	mem->keyTo = 0; // Mark the message as readed
	
	sem_post(sd);

	return out;
}

int 
sendMessage(NodeType to, Message * msg){

	Message * mem;	
	Message * aux;
	int fd, i;
	
	if(to == SERVER)
		fd = serverFd;	
	else
		fd = clientFd;
	printf("FD: %d. PID: %d\n", fd, getpid());
	
	if ( !(mem = mmap(NULL, ANTQUANT*SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) )
		errorLog("mmap");

	aux = mem;
	sem_wait(sd);
	for(i = 1; mem->keyTo != 0; i++){ // Search for a space readed
		mem = aux+SIZE*(i%ANTQUANT);
		sem_post(sd);
		//sleep(1);
		printf("Buscando donde dejar mensaje\n");
		sem_wait(sd);
	}

	printf("Mensaje mandado a %d\n", i-1%ANTQUANT);
	sem_post(sd);

	// Block this zone of code
	sem_wait(sd);

	memcpy(mem, msg, SIZE);

	sem_post(sd);

	return 0;
}
/*
int
getid(int pid){
	if(_vecPid == NULL)
		_vecPid = calloc(ANTQUANT*sizeof(int));

	int i, ans = -1;
	for(i = 0; i < ANTQUANT; i++)
		if(_vecPid[i] == 0 || _vecPid[i] == pid){
			_vecPid[i] = pid;
			ans = i;
			break;
		}
	return ans;
}*/
