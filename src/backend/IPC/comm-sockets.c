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
#define CLT_KEY_BASE 3	// Map key: 1. Anthill key: 2. Clients keys are > 2

int clientCount = 0;

// Socket descriptors
int serverSd;
int * clientSds;
int * acceptedClients;

// Socket structures
struct sockaddr_in serverSide;
struct sockaddr_in clientSide;

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

void openServer(void *t){
	int i, optionValue = 1;
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
	for (i=CLT_KEY_BASE-1; i<clientCount; i++)
		toRead[i].fd = INVALID_SOCKET;

	pthread_t acceptClientThread;
	pthread_create(&acceptClientThread, NULL, acceptClients, NULL);

}

void openClient(void *t){
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

// Close IPC resource
void closeIPC(){
	if ( close(serverSd) == -1 )
		errorLog("Server's socket descriptor could not be closed.");
}

// Eliminar closeIPC cuando esté la interfaz hecha para closeServer y closeCLient. Ahora adelanto esto.
void closeServer(){
	int i;
	if ( close(serverSd) == -1 )
		errorLog("Failed to close server socket descriptor.");

	for (i=CLT_KEY_BASE-1; i<clientCount; i++)
		if (close(acceptedClients[i]) == -1)
			errorLog("Failed to close accepted client socket descriptor.");
}

void closeClient(){
	int i;
	for (i=CLT_KEY_BASE-1; i<clientCount; i++)
		if (close(clientSds[i]) == -1)
			errorLog("Failed to close client socket descriptor.");
}

Message * receiveMessage(NodeType from, int key){
	Message * out = malloc(sizeof (Message));

	int sd, i, polled;
	if (from == CLIENT){	// I'm SERVER and I receive from CLIENT.
		polled = poll(toRead, clientCount, 5000000);

		if (polled == 0)
			errorLog("SERVER: poll() timed out.");
		else if (polled < 0)
			errorLog("SERVER: Failed to poll() connections.");

		/* See which socket is the one ready to be read. */
		for (i = CLT_KEY_BASE-1; i<clientCount; i++){
			if (toRead[i].revents & POLLIN){
				sd = toRead[i].fd;
				toRead[i].revents = 0;
				break;
			}
		}

		if ( recv(sd, out, sizeof(Message), 0) < 0)
			errorLog("SERVER: Failed to receive data from client.");

	} else {		// I'm CLIENT and I receive from SERVER.
		if ( recv(clientSds[key], out, sizeof(Message), 0) < 0)
			errorLog("CLIENT: Failed to receive data from server.");
	}

	return out;
}

int sendMessage(NodeType to, Message * msg){
	if (to == CLIENT){	// I'm SERVER and I send to CLIENT.	
		if ( send(acceptedClients[msg->keyTo], msg, sizeof(Message), 0) < 0)
			errorLog("SERVER: Failed to send data to client.");

	} else			// I'm CLIENT and I send to SERVER.
		if ( send(clientSds[msg->keyFrom], msg, sizeof(Message), 0) < 0)
			errorLog("CLIENT: Failed to send data to server.");

	return 0;
}

