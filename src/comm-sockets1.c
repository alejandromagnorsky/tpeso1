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
#include "../include/communication.h"

#define LOCALHOST INADDR_LOOPBACK
#define SERVER_IP LOCALHOST
#define SERVER_PORT 8888

#define CLIENTQUANT 20		// I DO need this.

// Socket descriptors
int serverSd;
int * clientSds; //[CLIENTQUANT];
int * acceptedClients;//[CLIENTQUANT];

// Socket structures
struct sockaddr_in serverSide;
struct sockaddr_in clientSide;

struct pollfd * toRead;

int digits(int n){
	return ((n/10)==0) ? 1 : 1 + digits(n/10);
}

void sigHandler(){
	destroyIPC();
	exit(1);
}

// Destroy IPC resources
void destroyIPC(){

}

// Open & initialize IPC resource
void openServer(void *t){

	printf("Server\n");
	int i, optionValue = 1;

	/* Fill serverSide structure */
	memset(&serverSide, 0, sizeof(struct sockaddr_in));	/* Zeroes struct */
	serverSide.sin_family = AF_INET;			/* IP protocol */
	serverSide.sin_addr.s_addr = INADDR_ANY;  		/* Server IP address */
	serverSide.sin_port = htons(SERVER_PORT);		/* Server port */

	/* Fill clientSide structure */
	memset(&clientSide, 0, sizeof(struct sockaddr_in));
	clientSide.sin_family = AF_INET;
	clientSide.sin_addr.s_addr = htonl(SERVER_IP);
	clientSide.sin_port = htons(SERVER_PORT);

	/* Create server socket descriptor */
	if ((serverSd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        	errorLog("Failed to create server socket descriptor");

	/* Allows resuing IP addresses */
	if (setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optionValue , sizeof(int)) < 0)
        	errorLog("Failed to reuse address");

	/* Bind server to the local address */
	if (bind(serverSd, (struct sockaddr *) &serverSide, sizeof(serverSide)) < 0)
        	errorLog("Failed to bind server");

	/* Mark server socket so it will listen for incoming connections */
	if (listen(serverSd, CLIENTQUANT) < 0)
		errorLog("Failed to listen to socket");

	printf("My pid=%d", getpid());
	clientSds = malloc(CLIENTQUANT * sizeof(int));
	toRead = malloc(CLIENTQUANT * sizeof(struct pollfd));
	
	acceptedClients = malloc(CLIENTQUANT * sizeof(int));

	for (i=0; i<CLIENTQUANT; i++){
		if ((clientSds[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		       	errorLog("Could not create client socket.");
		
		if (connect(clientSds[i], (struct sockaddr *) &clientSide, sizeof(clientSide)) < 0)
        		errorLog("Failed to connect to server.");

		if ((acceptedClients[i] = accept(serverSd, NULL, NULL)) < 0)
			errorLog("Failed to accept connection from client.");

		toRead[i].fd = acceptedClients[i];
		toRead[i].events = POLLIN;
		toRead[i].revents = 0;
	}

	printf("Termino el server\n");
}

void openClient(void *t){
	printf("Cliente\n");
/*	int i;
	for (i=0; i<CLIENTQUANT; i++){
		if ((clientSds[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	        	errorLog("Could not create client socket.");

		if (connect(clientSds[i], (struct sockaddr *) &clientSide, sizeof(clientSide)) < 0)
        		errorLog("Failed to connect to server.");

		if (accept(serverSd, NULL, NULL) < 0)
			errorLog("Failed to accept connection from client.");
	}*/
	printf("Termino cliente\n");
}

// Close IPC resource
void closeIPC(){
	if ( close(serverSd) == -1 )
		errorLog("Server's socket descriptor could not be closed.\n");
}



Message * receiveMessage(NodeType from, int key){
	Message * out = malloc(sizeof (Message));
	int sd;
	int polled;
	int i;
	for(i=0; i<CLIENTQUANT; i++){
		printf("clientSds(%d): %d  -  acceptedCLients(%d): %d\n", i, clientSds[i], i, acceptedClients[i]);
	}

	//printf("My pid=%d", getpid());
	if (from == CLIENT){	// I'm SERVER and I receive from CLIENT.
		polled = poll(toRead, CLIENTQUANT, 5000);
		printf("POLL: %d\n", polled);
		for (i = 0; i<polled && toRead[i].revents != POLLIN; i++){
			
		}
		
		if ( recv(toRead[i].fd, out, sizeof out, 0) < 0)
			errorLog("SERVER: Failed to receive data from client.");
		printf("SOY SERVER RECIBI DATA\n");
	} else {		// I'm CLIENT and I receive from SERVER.
		printf("SOY CLIENTE RECBIENDO DATA\n");
		if ( recv(clientSds[key], out, sizeof out, 0) < 0)
			errorLog("CLIENT: Failed to receive data from server.");
		printf("SOY CLIENTE RECBIENDO DATA\n");
	}

	return out;

}

int sendMessage(NodeType to, Message * msg){

	int i;
	for(i=0; i<CLIENTQUANT; i++){
		printf("clientSds(%d): %d  -  acceptedCLients(%d): %d\n", i, clientSds[i], i, acceptedClients[i]);
	}

	//printf("My pid=%d", getpid());
	printf("SOY CLIENTE? %d . Mi KEY ES: %d. Y MI SD ES: %d - ACCEPTED: %d\n", 1-to, msg->keyFrom+1, clientSds[msg->keyFrom], clientSds[msg->keyFrom]);
	if (to == CLIENT){	// I'm SERVER and I send to CLIENT.	
		printf("SOY SERVER MANDANDO DATA\n");
		if ( send(acceptedClients[msg->keyTo], msg, sizeof msg, 0) < 0)
			errorLog("SERVER: Failed to send data to client.");
		printf("SOY SERVER MANDE DATA\n");
	} else {		// I'm CLIENT and I send to SERVER.
		printf("SOY CLIENTE MANDANDO DATA\n");
		if ( send(clientSds[msg->keyFrom], msg, sizeof msg, 0) < 0)
			errorLog("CLIENT: Failed to send data to server.");
		printf("SOY CLIENTE MANDE DATA\n");
	}

	return 0;
}

