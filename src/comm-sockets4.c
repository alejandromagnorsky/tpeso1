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
		int slct;
		unsigned int clientFromSize;
		int clientSd;
		int i;
		struct sockaddr_in clientFrom;
		struct timeval time;

		memcpy(&aux_active_sd, &active_sd, sizeof(aux_active_sd));
		time.tv_sec = 30;
		time.tv_usec = 0;
		slct = select(maxsd + 1, &aux_active_sd, NULL, NULL, &time);
		if (!slct)
			errorLog("select() timed out.");
		else if (slct < 0 && errno != EINTR)
			errorLog("Failed to select from socket descriptor set.");
		else if (slct > 0) {
			if (FD_ISSET(serverSd, &aux_active_sd)) {
				clientFromSize = sizeof(clientFrom);
				if ((clientSd = accept(serverSd, (struct sockaddr *) &clientFrom, &clientFromSize)) < 0)
					errorLog("RECEIVE: Failed to accept client connection.");
				else {
					FD_SET(clientSd, &active_sd);
					maxsd = (maxsd < clientSd)?clientSd:maxsd;
				}
				FD_CLR(serverSd, &aux_active_sd);
			}
			for (i=0; i<maxsd+1; i++) {
				if (FD_ISSET(i, &aux_active_sd)) {
					printf("\t\t****************\n\t\tSOY SERVER Y ESTOY RECIBIENDO DE SD %d...\n", i);
					if ((slct = recv(i, out, sizeof(Message), 0)) < 0 ){
						errorLog("Failed to receive data from client.");
					} else if (slct == 0) {
						close(i);
						FD_CLR(i, &aux_active_sd);
					} else {
						printf("\t\tSOY SERVER Y RECIBI DE SD %d - keyFrom: %d.\n\t\t*****************\n", i, out->keyFrom);
						return out;
					}
					
					//printf("EL SD DEL CUAL RECIBO ES: %d || SOY SERVER (key: %d) Y RECIBÍ EL MENSAJE:\n", i, key);
					//printMessage(out);
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
		printf("************\nSOY CLIENT %d, MI CLIENTSD ES %d Y ESTOY RECIBIENDO DATA...\n", key, clientSds[key]);
		if (recv(clientSds[key], out, sizeof(out), 0) < 1)
                	errorLog("Failed to receive data from server.\n");
		printf("SOY CLIENT %d, MI CLIENTSD ES %d Y RECIBI DATA\n*****************\n", key, clientSds[key]);
	}
	return out;

}

int sendMessage(NodeType to, Message * msg){
	if (to == CLIENT){	// I'm SERVER and have to send to CLIENT.
		int slct;
		unsigned int clientFromSize;
		int clientSd;
		int i;
		struct sockaddr_in clientFrom;
		struct timeval time;

		memcpy(&aux_active_sd, &active_sd, sizeof(aux_active_sd));
		time.tv_sec = 30;
		time.tv_usec = 0;
		slct = select(maxsd + 1, &aux_active_sd, NULL, NULL, &time);
		if (!slct)
			errorLog("select() timed out.");
		else if (slct < 0 && errno != EINTR)
			errorLog("Failed to select from socket descriptor set.");
		else if (slct > 0) {
			if (FD_ISSET(serverSd, &aux_active_sd)) {
				clientFromSize = sizeof(clientFrom);
				if ((clientSd = accept(serverSd, (struct sockaddr *) &clientFrom, &clientFromSize)) < 0)
					errorLog("RECEIVE: Failed to accept client connection.");
				else {
					FD_SET(clientSd, &active_sd);
					maxsd = (maxsd < clientSd)?clientSd:maxsd;
				}
				FD_CLR(serverSd, &aux_active_sd);
			}
			for (i=0; i<maxsd+1; i++) {
				if (FD_ISSET(i, &aux_active_sd)) {
					printf("\t\t****************\n\t\tSOY SERVER Y ESTOY MANDANDO SD %d...\n", i);
					if ((slct = send(i, msg, sizeof(msg), 0)) < 0)
						errorLog("Failed to send data to client.");
					else if (slct == 0){
						close(i);
						FD_CLR(i, &aux_active_sd);
					} else {
						printf("\t\tSOY SERVER Y MANDE SD %d - keyTo: %d.\n\t\t*****************\n", i, msg->keyTo);
						return 0;
					}
					//printf("EL SD AL CUAL MANDO ES: %d || SOY SERVER Y ENVIÉ EL MENSAJE:\n", i);
					//printMessage(msg);
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
		printf("************\nSOY CLIENT %d, MI CLIENTSD ES %d Y ESTOY MANDANDO DATA...\n", msg->keyFrom, clientSds[msg->keyFrom]);
		if (send(clientSds[msg->keyFrom], msg, sizeof(msg), 0) < 1)
                	errorLog("Failed to send data to server.\n");	
		printf("************\nSOY CLIENT %d, MI CLIENTSD ES %d MANDE DATA...\n", msg->keyFrom, clientSds[msg->keyFrom]);
	}
	return 0;
}

