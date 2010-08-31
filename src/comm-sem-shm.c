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

Message * getmem(char * memKey);

static char * semKey = "/mutex";
static sem_t * sd;
static int fd; // For the shm

void 
sigHandler(){
	destroyIPC();
	exit(1);
}


void 
destroyIPC(){
	sem_unlink(semKey);
	shm_unlink("/server");
	shm_unlink("/ANT");	
}

void openIPC(){
	if ( !(sd = sem_open(semKey, O_RDWR|O_CREAT, 0666, 1)) ) // Create and initialize the semaphore if isn't exists
		errorLog("sem_open");
}


void
closeIPC(){

}


Message * 
receiveMessage(NodeType from){

	Message * mem;
	Message * out = NULL;
		
	char * memKey;
	if(from == SERVER){
		memKey = malloc(sizeof("/hormiga_") + sizeof(int));
		sprintf(memKey, "/hormiga_%d", getpid());
	}	
	else
		memKey = "/server";	
	
	mem = getmem(memKey);
	memset(mem, 0, SIZE);
	printf("Recibiendo\n");

	//Block the condition in the while, the assigment of out and the readed flag
	sem_wait(sd);
	while(mem->keyTo == 0){
		sem_post(sd);
		sleep(1);	
		//printf("%d recibe de la posicion: %lu\n", getpid(), (long)mem);	
		sem_wait(sd);
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
	
	char * memKey;
	if(to == SERVER)
		memKey = "/server";		
	else{
		memKey = malloc(sizeof("/hormiga_")+ sizeof(int));
		sprintf(memKey, "/hormiga_%d", msg->keyTo);
	}
	
	mem = getmem(memKey);
	memset(mem, 0, SIZE);
	
	// Block this zone of code
	sem_wait(sd);

	memcpy(mem, msg, SIZE);

	sem_post(sd);

	return 0;
}


Message *
getmem(char * memKey)
{
	
	Message * mem;
	
	if ( (fd = shm_open(memKey, O_RDWR|O_CREAT, 0666)) == -1 )
		errorLog("sh_open");
	ftruncate(fd, SIZE);
	printf("FD: %d. Memkey: %s\n", fd, memKey);
	if ( !(mem = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) )
		errorLog("mmap");
	close(fd);

	return mem;
}
