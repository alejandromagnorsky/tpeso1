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
#include "../include/communication.h"

#define SRV_FIFO "/tmp/server"
#define CLT_FIFO "/tmp/client_"
#define CLT_KEY_BASE 3	// Map key: 1. Anthill key: 2.

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


// Close IPC resource
void closeIPC(){
	if ( close(serverRD) == -1 )
		errorLog("Server's FIFO could not be closed.\n");
	int i;
	for (i=0; i<clientQuant; i++)
		if ( close(clientsWR[i]) == -1 )
			errorLog("A client's FIFO could not be closed.\n");
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
	for (i=CLT_KEY_BASE-1; i<clientQuant; i++){
		sprintf(fifoname, "%s%d", CLT_FIFO, i);
		if ( unlink(fifoname) < 0 )
              		errorLog("Clients' FIFOs could not be unlinked.");
	}
}

void createFifos(int clients){
	int i;
	int len = strlen(CLT_FIFO) + digits(clients-1);
	char * fifoname;
	if ( access(SRV_FIFO, 0) == -1 && mknod(SRV_FIFO, S_IFIFO | 0666, 0) == -1 )
                errorLog("Server's FIFO could not be created.");

        if ( (fifoname = malloc(len)) == NULL )
                errorLog("Memory allocation error in clients' FIFOs creation.");

        for (i=CLT_KEY_BASE-1; i<clients; i++){
                sprintf(fifoname, "%s%d", CLT_FIFO, i);
                if ( access(fifoname, 0) == -1 && mknod(fifoname, S_IFIFO | 0666, 0) == -1 )
                        errorLog("Clients' FIFOs could not be created.");
        }
}

////////////////////////////////////////////////////////////////////////////////////////
///////////    FUNCIONES TEMPORALES ANTES DE PROBAR MUTEX ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
// Funcion temporal 
void  * openServerFifos(void * t){
	int i;
	int len = strlen(CLT_FIFO) + digits(clientQuant-1);
	char * fifoname;
	clientsWR = malloc(clientQuant * sizeof(int));

	if ( (fifoname = malloc(len)) == NULL )
                errorLog("Memory allocation error in server' FIFO opening.");

	// I'm SERVER and I open my FIFO for reading purposes only. 
        if ( (serverRD = open(SRV_FIFO, O_RDONLY)) == -1 )
                errorLog("SERVER: Server's FIFO could not be opened.");

        for (i=CLT_KEY_BASE-1; i<clientQuant; i++){
                sprintf(fifoname, "%s%d", CLT_FIFO, i);
		// I'm SERVER and I open CLIENTs' FIFOS for writing purposes only. 
		if ( (clientsWR[i] = open(fifoname, O_WRONLY)) == -1 )
			errorLog("SERVER: Clients' FIFO could not be opened.\n");
        }
	pthread_exit(NULL);
}

// Funcion temporal 
void openServer(void * t){
	clientQuant = (int) t + CLT_KEY_BASE;

	createFifos(clientQuant);

	pthread_t serverFifoThread;
	pthread_create(&serverFifoThread, NULL, openServerFifos, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/* // NO BORRAR ESTA FUNCION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void openServer(void * t){
	printf("Empezó openServer\n");
	int i;
	clientQuant = (int) t + CLT_KEY_BASE;
	int len = strlen(CLT_FIFO) + digits(clientQuant-1);
	char * fifoname;
	clientsWR = malloc(clientQuant * sizeof(int));
 	
	createFifos(clientQuant);

	if ( (fifoname = malloc(len)) == NULL )
                errorLog("Memory allocation error in server' FIFO opening.");

	printf("\t Empezó el serverRd = open(). clientQuant -> %d. SRV_FIFO: %s. serverRD: %d\n", clientQuant, SRV_FIFO, serverRD);
//	// I'm SERVER and I open my FIFO for reading purposes only. 
        if ( (serverRD = open(SRV_FIFO, O_RDONLY)) == -1 )
                errorLog("SERVER: Server's FIFO could not be opened.");
	printf("\t Terminó el serverRd = open(). clientQuant -> %d. SRV_FIFO: %s\n", clientQuant, SRV_FIFO);

        for (i=CLT_KEY_BASE-1; i<clientQuant; i++){
                sprintf(fifoname, "%s%d", CLT_FIFO, i);
//		// I'm SERVER and I open CLIENTs' FIFOS for writing purposes only. 
//              if ( (clientsWR[i] = open(fifoname, O_WRONLY)) == -1 )
//			errorLog("SERVER: Clients' FIFO could not be opened.");
        }
	printf("Terminó openServer\n");
}*/

void openClient(void * t){
	int i;
	clientQuant = (int) t + CLT_KEY_BASE;
	int len = strlen(CLT_FIFO) + digits(clientQuant-1);
	char * fifoname;	
	clientsRD = malloc(clientQuant * sizeof(int));
 	
	createFifos(clientQuant);

	if ( (fifoname = malloc(len)) == NULL )
                errorLog("Memory allocation error in clients' FIFOs opening.");

	/* I'm CLIENT and I open SERVER's FIFO for writing purposes only. */
        if ( (serverWR = open(SRV_FIFO, O_WRONLY)) == -1 )
                errorLog("CLIENT: Server's FIFO could not be opened.");

        for (i=CLT_KEY_BASE-1; i<clientQuant; i++){
                sprintf(fifoname, "%s%d", CLT_FIFO, i);
		/* I'm CLIENT and I open CLIENTS' FIFOS for reading purposes only. */
		if ( (clientsRD[i] = open(fifoname, O_RDONLY)) == -1 )
			errorLog("CLIENT: Clients' FIFO could not be opened.");
        }
}

Message * receiveMessage(NodeType from, int key){
	int fd;
	Message * out = malloc(sizeof(Message));

	if (from == CLIENT)	// I'm SERVER and have to read from my FIFO.
		fd = serverRD;
	else
		fd = clientsRD[key];	// I'm CLIENT and have to read from my FIFO.
	
//	printf("I'M %s READING...\n", (from==CLIENT)?"SERVER":"CLIENT" );
	read(fd, out, sizeof(Message));
//	printf("I'M %s . I FINISH READING\n", (from==CLIENT)?"SERVER":"CLIENT" );
	return out;
}

int sendMessage(NodeType to, Message * msg){
	int fd;
	if (to == SERVER)	// I'm CLIENT and have to write in SERVER's FIFO.
		fd = serverWR;
	else 
		fd = clientsWR[msg->keyTo];	// I'm SERVER and have to write in CLIENT's FIFO.

//	printf("I'M %s WRITING...\n", (to==SERVER)?"CLIENT":"SERVER" );
	write(fd, msg, sizeof(Message));
//	printf("I'M %s. I FINISHED WRITING\n", (to==SERVER)?"CLIENT":"SERVER" );
	return 0;
}
