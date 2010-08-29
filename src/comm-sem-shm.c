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
void fatal(char *s);
void initmutex(void);

static char * semKey = "/mutex";
static sem_t * sd;


void 
sigHandler(){
	closeNode(0);
	exit(1);
}


void 
closeNode(NodeType t){
	sem_unlink(semKey);
	if(t == MAP)
		shm_unlink("/map");
}


Message * 
receiveMessage(NodeType from){

	Message * mem;
	Message * out = NULL;
		
	char * memKey;
	if(from == MAP){
		memKey = malloc(sizeof("hormiga_") + sizeof(int));
		sprintf(memKey, "hormiga_%d", getpid());
	}	
	else
		memKey = "/map";	
	
	initmutex();	// Initialize the semaphore if isn't exists
	mem = getmem(memKey);
	memset(mem, 0, SIZE);
	printf("Recibiendo\n");

	//Block the condition in the while, the assigment of out and the readed flag
	sem_wait(sd);
	while(mem->pidTo == 0){
		sem_post(sd);
		sleep(1);	
		//printf("%d recibe de la posicion: %lu\n", getpid(), (long)mem);	
		sem_wait(sd);
	}
	
	// A copy must be made, because mem is deallocated after this function
	out = createMessage(mem->pidFrom, mem->pidTo, mem->opCode,  mem->param, mem->pos, mem->trace);

	mem->pidTo = 0; // Mark the message as readed
	
	sem_post(sd);

	return out;
}

int 
sendMessage(NodeType to, Message * msg){

	Message * mem;	
	
	char * memKey;
	if(to == MAP)
		memKey = "/map";		
	else{
		memKey = malloc(sizeof("hormiga_")+ sizeof(int));
		sprintf(memKey, "hormiga_%d", msg->pidTo);
	}


	mem = getmem(memKey);
	memset(mem, 0, SIZE);
	initmutex(); // Initialize the semaphore if isn't exists
	
	// Block this zone of code
	sem_wait(sd);

	memcpy(mem, msg, SIZE);

	sem_post(sd);

	return 0;
}


void
initmutex(void)
{
	if ( !(sd = sem_open(semKey, O_RDWR|O_CREAT, 0666, 1)) )
		fatal("sem_open");
}

Message *
getmem(char * memKey)
{
	int fd;
	Message * mem;
	
	if ( (fd = shm_open(memKey, O_RDWR|O_CREAT, 0666)) == -1 )
		fatal("sh_open");
	ftruncate(fd, SIZE);

	if ( !(mem = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) )
		fatal("mmap");
	close(fd);

	return mem;
}

void
fatal(char *s)
{
	perror(s);
	exit(1);
}
