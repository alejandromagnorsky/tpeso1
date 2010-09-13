#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#include "../../../include/transport.h"
#include "../../../include/communication.h"


static key_t SERVERKey = 1235;

static int queueID = 0;

void sigHandler(){
	destroyIPC();
	exit(1);
}

void destroyIPC(){
	msgctl(queueID, IPC_RMID, NULL); 
}

// Open & initialize IPC resource
void queue_open(){
	if ( ( queueID = msgget(SERVERKey,O_RDONLY | O_CREAT | IPC_CREAT |0666)) == -1 )
		errorLog("Could not create message queue");
}

void openServer(void * t, int size){
	queue_open();
}

void openClient(void * t, int size){
	queue_open();
}

void closeServer(){
	destroyIPC();
}

void closeClient(){
}

int receiveFromServer( int key, char * buf, int size ){

	// This is the equivalent of msgbuf struct 
	char * data = malloc(size + sizeof(long int));

	int out = msgrcv(queueID, data, size, key, 0);

	// Now copy data and point buf to it
	memcpy(buf, data + sizeof(long int), size);

	// Free data already copied
	free(data);
	return out;
}

int receiveFromClient( int key, char * buf, int size ){
	

	// This is the equivalent of msgbuf struct 
	char * data = malloc(size + sizeof(long int));

	int out = msgrcv(queueID, data, size, 1, 0);

	// Now copy data and point buf to it
	memcpy(buf, data + sizeof(long int), size);

	// Free data already copied
	free(data);
	return out;
}

int sendToServer( int key, char * buf, int size ){

	char * data = malloc(sizeof(long int) + size);

	memcpy(data+sizeof(long int), buf, size);	// Data 

	long int * type = (long int *)data;
	*type = 1; // Priority

	int out = msgsnd(queueID, data, size, 0);

	// Free data, already sent 
	free(data);
	return out;
}

int sendToClient( int key, char * buf, int size ){

	char * data = malloc(sizeof(long int) + size);

	memcpy(data+sizeof(long int), buf, size);	// Data 

	long int * type = (long int *)data;
	*type = key; // Priority

	int out = msgsnd(queueID, data, size, 0);

	// Free data, already sent 
	free(data);
	return out;
}
