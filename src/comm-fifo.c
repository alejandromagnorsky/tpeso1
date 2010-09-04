#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include "../include/communication.h"

#define CLIENTQUANT 20		// I DO need this.

// FIFOs file descriptors
int serverFd; 
int clientFds[CLIENTQUANT];

int digits(int n){
	return ((n/10)==0) ? 1 : 1 + digits(n/10);
}

void sigHandler(){
	destroyIPC();
	exit(1);
}

// Destroy IPC resources
void destroyIPC(){
	if ( unlink("/server") == -1 )
                errorLog("Server's FIFO could not be unlinked.\n");
	int i;
	char * clientfifo;
	if ( (clientfifo = malloc(8 + digits(CLIENTQUANT-1))) == NULL )	// 8 = strlen("/client_")
		errorLog("Memory allocation error in client's FIFO creation.\n");
	for (i=0; i<CLIENTQUANT; i++){
		sprintf(clientfifo, "/client_%d", i);
		if ( unlink(clientfifo) == -1 )
              		errorLog("A client's FIFO could not be unlinked.\n");
	}
}

// Open & initialize IPC resource
void openIPC(){
	if ( access("/server", 0) == -1 && mknod("/server", S_IFIFO | 0666, 0) == -1 )
		errorLog("Server's FIFO could not be created.\n");
	if ( (serverFd = open("/server", O_RDWR)) == -1 )
		errorLog("Server's FIFO could not be opened.\n");

	char * clientfifo;
	if ( (clientfifo = malloc(8 + digits(CLIENTQUANT-1))) == NULL )	// 8 = strlen("/client_")
		errorLog("Memory allocation error in client's FIFO creation.\n");
	int i;
	for (i=0; i<CLIENTQUANT; i++){
		sprintf(clientfifo, "/client_%d", i);
		if ( access(clientfifo, 0) == -1 && mknod(clientfifo, S_IFIFO | 0666, 0) == -1 )
			errorLog("Client's FIFO could not be created.\n");
		if ( (clientFds[i] = open(clientfifo, O_RDWR)) == -1 )
			errorLog("Client's FIFO could not be opened.\n");
	}
}

// Close IPC resource
void closeIPC(){
	if ( close(serverFd) == -1 )
		errorLog("Server's FIFO could not be closed.\n");
	int i;
	for (i=0; i<CLIENTQUANT; i++)
		if ( close(clientFds[i]) == -1 )
			errorLog("A client's FIFO could not be closed.\n");
}

Message * receiveMessage(NodeType from, int key){
	int fd;
	Message * out = malloc(sizeof(Message));

	if (from == CLIENT)	// I'm server and have to read from my FIFO.
		fd = serverFd;
	else
		fd = clientFds[key];	// I'm client and have to read from my portion of FIFO.

	read(fd, out, sizeof(Message));
	return out;
}

int sendMessage(NodeType to, Message * msg){
	int fd;

	if (to == SERVER)	// I'm client and have to write in server's FIFO.
		fd = serverFd;
	else 
		fd = clientFds[msg->keyTo];	// I'm server and have to write in client's FIFO.

	write(fd, msg, sizeof(Message));
	return 0;
}
