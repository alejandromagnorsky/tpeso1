#include <stdio.h>
#include <string.h>
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
#include <pthread.h>
#include "../../../include/communication.h"

#define SRV_FIFO "/tmp/server"
#define CLT_FIFO "/tmp/client_"
#define CLT_KEY_BASE 2	// Map key: 1. Anthill key: 2. Clients keys are > 2

int clientQuant = 0;

// FIFOs file descriptors
int serverRD;
int serverWR;
int * clientsRD;
int * clientsWR;

int digits(int n){
	return ((n/10)==0) ? 1 : 1 + digits(n/10);
}

void sigHandler(){
	destroyIPC();
	exit(1);
}

void closeServer(void * t, int size){
	int i;
	if ( close(serverRD) == -1 )
		errorLog("Server's RDONLY FIFO could not be closed.");

	for (i=0; i<clientQuant; i++)
		if ( close(clientsWR[i]) == -1 )
			errorLog("Clients' WRONLY FIFOs could not be closed.");
	free(clientsWR);
}

void closeClient(void * t, int size){
	int i;
	if ( close(serverWR) == -1 )
		errorLog("Server's WRONLY FIFO could not be closed.");

	for (i=0; i<clientQuant; i++)
		if ( close(clientsRD[i]) == -1 )
			errorLog("Clients' RDONLY FIFOs could not be closed.");
	free(clientsRD);
}

// Destroy IPC resources
void destroyIPC(){	
	int i;
	int len = strlen(CLT_FIFO) + digits(clientQuant-1);
	char * fifoname;

	if ( unlink(SRV_FIFO) < 0 )
                errorLog("Server's FIFO could not be unlinked.");
	if ( (fifoname = malloc(len)) == NULL )
		errorLog("Memory allocation error in client's FIFO deletion.");
	for (i=0; i<clientQuant; i++){
		sprintf(fifoname, "%s%d", CLT_FIFO, i);
		if ( unlink(fifoname) < 0 )
              		errorLog("Clients' FIFOs could not be unlinked.");
	}
	free(fifoname);
}

void createFifos(int clients){
	int i;
	int len = strlen(CLT_FIFO) + digits(clients-1);
	char * fifoname;
	if ( access(SRV_FIFO, 0) == -1 && mknod(SRV_FIFO, S_IFIFO | 0666, 0) == -1 )
                errorLog("Server's FIFO could not be created.");

        if ( (fifoname = malloc(len)) == NULL )
                errorLog("Memory allocation error in clients' FIFOs creation.");

        for (i=0; i<clients; i++){
                sprintf(fifoname, "%s%d", CLT_FIFO, i);
                if ( access(fifoname, 0) == -1 && mknod(fifoname, S_IFIFO | 0666, 0) == -1 )
                        errorLog("Clients' FIFOs could not be created.");
        }
	free(fifoname);
}

void * openServerFifos(void * t){
	int i;
	int len = strlen(CLT_FIFO) + digits(clientQuant-1);
	char * fifoname;
	clientsWR = malloc(clientQuant * sizeof(int));

	if ( (fifoname = malloc(len)) == NULL  || clientsWR == NULL)
                errorLog("Memory allocation error in server' FIFO opening.");

	/* I'm SERVER and I open my FIFO for reading purposes only. */
        if ( (serverRD = open(SRV_FIFO, O_RDONLY)) == -1 )
                errorLog("SERVER: Server's FIFO could not be opened.");

        for (i=0; i<clientQuant; i++){
                sprintf(fifoname, "%s%d", CLT_FIFO, i);
		/* I'm SERVER and I open CLIENTs' FIFOS for writing purposes only. */
		if ( (clientsWR[i] = open(fifoname, O_WRONLY)) == -1 )
			errorLog("SERVER: Clients' FIFO could not be opened.");
        }
	free(fifoname);
	pthread_exit(NULL);
}

void openServer(void * t, int size){
	clientQuant = (int) t + 1;	// Clients quantity (ants + anthill)

	createFifos(clientQuant);

	pthread_t serverFifoThread;
	pthread_create(&serverFifoThread, NULL, openServerFifos, NULL);
}

void openClient(void * t, int size){
	int i;
	clientQuant = (int) t + 1;	// Clients quantity (ants + anthill)
	int len = strlen(CLT_FIFO) + digits(clientQuant-1);
	char * fifoname;	
	clientsRD = malloc(clientQuant * sizeof(int));
 	
	createFifos(clientQuant);

	if ( (fifoname = malloc(len)) == NULL || clientsRD == NULL )
                errorLog("Memory allocation error in clients' FIFOs opening.");

	/* I'm CLIENT and I open SERVER's FIFO for writing purposes only. */
        if ( (serverWR = open(SRV_FIFO, O_WRONLY)) == -1 )
                errorLog("CLIENT: Server's FIFO could not be opened.");

        for (i=0; i<clientQuant; i++){
                sprintf(fifoname, "%s%d", CLT_FIFO, i);
		/* I'm CLIENT and I open CLIENTS' FIFOS for reading purposes only. */
		if ( (clientsRD[i] = open(fifoname, O_RDONLY)) == -1 )
			errorLog("CLIENT: Clients' FIFO could not be opened.");
        }
	free(fifoname);
}

int receiveFromClient(int key, char * buf, int size){
	int bytes;
	bytes = read(serverRD, buf, size);
	return bytes;
}

int receiveFromServer(int key, char * buf, int size){
	int bytes;
	bytes = read(clientsRD[key-CLT_KEY_BASE], buf, size);
	return bytes;
}

int sendToClient(int key, char * buf, int size){
	int bytes;
	bytes = write(clientsWR[key-CLT_KEY_BASE], buf, size);

	return bytes;
}

int sendToServer(int serverKey, char * buf, int size){
	int bytes;
	bytes = write(serverWR, buf, size);

	return bytes;
}
