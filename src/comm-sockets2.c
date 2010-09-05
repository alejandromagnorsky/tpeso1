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

#include "../include/communication.h"

#define LOCALHOST INADDR_LOOPBACK
#define SERVER_IP LOCALHOST
#define SERVER_PORT 6300

#define CLIENTQUANT 20		// I DO need this.

// Server socket descriptor
int serverSd;
int clientSds[CLIENTQUANT];

// Max server socket descriptor
int maxsd;

// Socket descriptors set
fd_set active_sd;
fd_set aux_active_sd;

// Socket structures
struct sockaddr_in serverSide;
struct sockaddr_in clientSide;

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
void openIPC(){
	int optionValue = 1;
	int i;
	
	for (i=0; i<CLIENTQUANT; i++){
		clientSds[i] = -1;
	}

	/* Fill serverSide structure */
	memset(&serverSide, 0, sizeof(struct sockaddr_in));	/* Zeroes struct */
	serverSide.sin_family = AF_INET;		/* IP protocol */
	serverSide.sin_addr.s_addr = htonl(INADDR_ANY);  	/* Server IP address */
	serverSide.sin_port = htons(SERVER_PORT);	/* Server port */

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
	
	/* Initialize active_sd set */
	FD_ZERO(&active_sd);
	FD_SET(serverSd, &active_sd);
	maxsd = serverSd;
	
}

// Close IPC resource
void closeIPC(){
	if ( close(serverSd) == -1 )
		errorLog("Server's socket descriptor could not be closed.\n");
}



Message * receiveMessage(NodeType from, int key){
	Message * out = malloc(sizeof(Message));

	if (from == CLIENT){	// I'm SERVER and have to receive from CLIENT.
		unsigned int clientFromSize;
		int i;
		int rec;
		struct sockaddr_in clientFrom;

		aux_active_sd = active_sd;
       		if (select (maxsd+1, &aux_active_sd, NULL, NULL, NULL) < 0)
			perror ("Select() failed.");

		for (i = 0; i < FD_SETSIZE; i++){
             		if (FD_ISSET (i, &aux_active_sd)){
                 		if (i == serverSd){
                     			/* Connection request on original socket. */
                     			int new;
                     			clientFromSize = sizeof (clientFrom);
                     			new = accept (serverSd, (struct sockaddr *) &clientFrom, &clientFromSize);
                    			if (new < 0)
						errorLog ("RECIVE: Failed to accept incoming connections");
					FD_SET (new, &active_sd);
                   		} else {
                     			/* Data arriving on an already-connected socket. */
					printf("SOY SERVER Y ESTOY RECIBIENDO DATA DEL SD %d\n", i);
					do {
						rec = recv(i, out, sizeof(out), 0);
					} while (rec == -1 && errno == EINTR);

                     			if (rec < 0){
	                         		close(i);
                         			FD_CLR(i, &active_sd);
						errorLog("RECEIVE: Failed to receive from CLIENT.");
					} else if (rec == 0){
						close(i);
                         			FD_CLR(i, &active_sd);
						errorLog("RECEIVED: 0 bytes recived from client.");
					}
					printf("SOY SERVER Y RECIBI DATA DEL SD %d\n", i);
					return out;
				}
			}
		}

	} else {		// I'm CLIENT and have to receive from SERVER.
		if (clientSds[key] == -1){
			if ((clientSds[key] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				errorLog("Failed to create client socket descriptor.");
	
			if (connect(clientSds[key], (struct sockaddr *) &clientSide, sizeof(clientSide)) < 0)
	             		errorLog("Failed to connect client socket to the server.");	
		}
		
		printf("SOY CLIENT %d Y ESTOY RECIBIENDO DATA...\n", key);
		if (recv(clientSds[key], out, sizeof(out), 0) < 1)
                	errorLog("Failed to receive data from server.\n");
		printf("SOY CLIENTE %d Y RECIBI DATA\n", key);

		close(clientSds[key]);
		clientSds[key] = -1;
	}
	return out;

}

int sendMessage(NodeType to, Message * msg){
	if (to == CLIENT){	// I'm SERVER and have to send to CLIENT.
		unsigned int clientFromSize;
		int i;
		struct sockaddr_in clientFrom;

		aux_active_sd = active_sd;
       		if (select (maxsd+1, &aux_active_sd, NULL, NULL, NULL) < 0)
			perror ("Select() failed.");

		for (i = 0; i < FD_SETSIZE; ++i){
             		if (FD_ISSET (i, &aux_active_sd)){
                 		if (i == serverSd){
                     			/* Connection request on original socket. */
                     			int new;
                     			clientFromSize = sizeof (clientFrom);
                     			new = accept (serverSd, (struct sockaddr *) &clientFrom, &clientFromSize);
                    			if (new < 0)
						errorLog ("SEND: Failed to accept incoming connections");
					FD_SET (new, &active_sd);
                   		} else {
                     			/* Data arriving on an already-connected socket. */
					printf("SOY SERVER Y ESTOY MANDANDO DATA AL SD %d\n", i);
                     			if (send(i, msg, sizeof(msg), 0) < 0){
	                         		close(i);
                         			FD_CLR(i, &active_sd);
						errorLog("RECEIVE: Failed to receive from CLIENT.");
					}
					printf("SOY SERVER Y MANDE DATA AL SD %d\n", i);
					return 0;
				}
			}
		}
	} else {		// I'm CLIENT and have to send to SERVER.
		if (clientSds[msg->keyFrom] == -1){
			if ((clientSds[msg->keyFrom] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				errorLog("Failed to create client socket descriptor.");
	
			if (connect(clientSds[msg->keyFrom], (struct sockaddr *) &clientSide, sizeof(clientSide)) < 0)
	             		errorLog("Failed to connect client socket to the server.");	
		}
		printf("SOY CLIENT %d Y ESTOY MANDANDO DATA...\n", msg->keyFrom);
		if (send(clientSds[msg->keyFrom], msg, sizeof(msg), 0) < 1)
                	errorLog("Failed to receive data from server.\n");
		printf("SOY CLIENTE %d Y MANDE DATA\n", msg->keyFrom);

		close(clientSds[msg->keyFrom]);
		clientSds[msg->keyFrom] = -1;
	}
	return 0;
}

