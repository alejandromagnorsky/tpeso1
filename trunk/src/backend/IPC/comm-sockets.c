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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <poll.h>
#include <pthread.h>
#include "../../../include/communication.h"

#define LOCALHOST INADDR_LOOPBACK
#define SERVER_IP LOCALHOST
#define SERVER_PORT 9000
#define INVALID_SOCKET -1
#define CLT_KEY_BASE 3	// Map key: -1. Anthill key: 2. Clients keys are > 2

int clientCount = 0;

// Socket descriptors
int serverSd;
int * clientSds;
int * acceptedClients;

// poll() structure
struct pollfd * toRead;

void sigHandler(){
	destroyIPC();
	exit(1);
}

// Destroy IPC resources
void destroyIPC(){

}

void * acceptClients(void * t){
	int i;
	for (i=CLT_KEY_BASE-1; i<clientCount; i++){
		if ((acceptedClients[i] = accept(serverSd, NULL, NULL)) < 0)
			errorLog("Failed to accept connection from client.");

		toRead[i].fd = acceptedClients[i];
		toRead[i].events = POLLIN;
		toRead[i].revents = 0;
	}

	pthread_exit(NULL);
}

void openServer(void * t, int size){
	int i, optionValue = 1;
	struct sockaddr_in serverSide;
	clientCount = (int) t + CLT_KEY_BASE;

	/* Fill serverSide structure */
	memset(&serverSide, 0, sizeof(struct sockaddr_in));	/* Zeroes struct */
	serverSide.sin_family = AF_INET;			/* IP protocol */
	serverSide.sin_addr.s_addr = INADDR_ANY;  		/* Server IP address */
	serverSide.sin_port = htons(SERVER_PORT);		/* Server port */

	/* Create server socket descriptor */
	if ((serverSd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        	errorLog("Failed to create server socket descriptor.");

	/* Allows reusing IP addresses */
	if (setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optionValue , sizeof(int)) < 0)
        	errorLog("Failed to reuse address.");

	/* Bind server to the local address */
	if (bind(serverSd, (struct sockaddr *) &serverSide, sizeof(serverSide)) < 0)
        	errorLog("Failed to bind server.");

	/* Mark server socket so it will listen for incoming connections */
	if (listen(serverSd, clientCount) < 0)
		errorLog("Failed to listen to socket.");

	acceptedClients = malloc(clientCount * sizeof(int));

	/* Initializes poll() struct */	
	toRead = malloc(clientCount * sizeof(struct pollfd));
	for (i=0; i<clientCount; i++)
		toRead[i].fd = INVALID_SOCKET;

	pthread_t acceptClientThread;
	pthread_create(&acceptClientThread, NULL, acceptClients, NULL);

}

void openClient(void *t, int size){
	struct sockaddr_in clientSide;
	clientCount = (int) t + CLT_KEY_BASE;
	
	/* Fill clientSide structure */
	memset(&clientSide, 0, sizeof(struct sockaddr_in));
	clientSide.sin_family = AF_INET;
	clientSide.sin_addr.s_addr = htonl(SERVER_IP);
	clientSide.sin_port = htons(SERVER_PORT);

	int i;
	clientSds = malloc(clientCount * sizeof(int));

	for (i=CLT_KEY_BASE-1; i<clientCount; i++){
		if ((clientSds[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		       	errorLog("Could not create client socket.");

		if (connect(clientSds[i], (struct sockaddr *) &clientSide, sizeof(clientSide)) < 0)
        		errorLog("Failed to connect to server.");
	}
}

void closeServer(void * t, int size){
	int i;
	if ( close(serverSd) == -1 )
		errorLog("Failed to close server socket descriptor.");

	for (i=CLT_KEY_BASE-1; i<clientCount; i++)
		if (close(acceptedClients[i]) == -1)
			errorLog("Failed to close accepted client socket descriptor.");
}

void closeClient(void * t, int size){
	int i;
	for (i=CLT_KEY_BASE-1; i<clientCount; i++)
		if (close(clientSds[i]) == -1)
			errorLog("Failed to close client socket descriptor.");
}

int receiveFromServer(int key, char * buf, int size){
	int bytes;
	char * msg = malloc(size);

	if ( (bytes = recv(clientSds[key], msg, size, 0)) < 0)
		errorLog("Failed to receive data from server.");

	memcpy(buf, msg, size);
	free(msg);
	return bytes;
}

int receiveFromClient(int serverKey, char * buf, int size){
	int sd, i, polled, bytes;
	char * msg = malloc(size);

	polled = poll(toRead, clientCount, 5000000);
	if (polled == 0)
		errorLog("poll() timed out.");
	else if (polled < 0)
		errorLog("Failed to poll() connections.");

	/* See which socket is the one ready to be read. */
	for (i = 0; i<clientCount; i++){
		if (toRead[i].revents & POLLIN){
			sd = toRead[i].fd;
			toRead[i].revents = 0;
			break;
		}
	}
	if ( (bytes = recv(sd, msg, size, 0)) < 0)
		errorLog("Failed to receive data from client.");
;
	memcpy(buf, msg, size);
	free(msg);
	return bytes;
}

int sendToServer(int key, char * buf, int size){
	int bytes;
	char * msg = malloc(size);

	memcpy(msg, buf, size);
	if ( (bytes = send(clientSds[key], msg, size, 0)) < 0)
		errorLog("Failed to send data to server.");

	free(msg);
	return bytes;
}

int sendToClient(int key, char * buf, int size){
	int bytes;
	char * msg = malloc(size);

	memcpy(msg, buf, size);
	if ( (bytes = send(acceptedClients[key], msg, size, 0)) < 0)
		errorLog("Failed to send data to client.");

	free(msg);
	return bytes;
}

