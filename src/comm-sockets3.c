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
int listener;
int clientSds[CLIENTQUANT];

// Max server socket descriptor
int maxsd;

// Socket descriptors set
fd_set master_set;
fd_set read_set;
fd_set write_set;

// Socket structures
struct sockaddr_in serverSide;
struct sockaddr_in clientSide;

int sendAll(int sd, Message * msg, int * msglen){
	int totalsent = 0;        // how many bytes we've sent
	int leftTosend = *msglen; // how many we have left to send
	int n;

	while(totalsent < *msglen) {
		if ( (n = send(sd, msg+totalsent, leftTosend, 0)) < 0)
			break;
		totalsent += n;
		leftTosend -= n;
	}

	*msglen = totalsent;
	return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
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
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        	errorLog("Failed to create server socket descriptor");

	/* Allows resuing IP addresses */
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const void *)&optionValue , sizeof(int)) < 0)
        	errorLog("Failed to reuse address");

	/* Bind server to the local address */
	if (bind(listener, (struct sockaddr *) &serverSide, sizeof(serverSide)) < 0)
        	errorLog("Failed to bind server");

	/* Mark server socket so it will listen for incoming connections */
	if (listen(listener, CLIENTQUANT) < 0)
		errorLog("Failed to listen to socket");
	
	/* Initialize sets */
	FD_ZERO(&master_set);
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_SET(listener, &master_set);
	maxsd = listener;
	
}

// Close IPC resource
void closeIPC(){
	if ( close(listener) == -1 )
		errorLog("Server's socket descriptor could not be closed.\n");
}



Message * receiveMessage(NodeType from, int key){
	Message * out = malloc(sizeof(Message));

	if (from == CLIENT){	// I'm SERVER and have to receive from CLIENT.
		int bytes;
		unsigned int clientFromSize;
		int clientSd;
		int i;
		struct sockaddr_in clientFrom;

		read_set = master_set; // Copy
		if ( select(maxsd + 1, &read_set, NULL, NULL, NULL) < 0 )
			errorLog("RECEIVE: Failed to select socket.");

		for (i=0; i<maxsd+1; i++){
			if (FD_ISSET(i, &read_set)) {
				if (i == listener) {
                   			// New incomming connection
                    			clientFromSize = sizeof clientFrom;
                  			if ( (clientSd = accept(listener, (struct sockaddr *)&clientFrom, &clientFromSize)) == -1)
                        			errorLog("RECEIVE: Failed to accept client connection.");
					else {
						FD_SET(clientSd, &master_set); // add to master set
						maxsd = (clientSd > maxsd)?clientSd:maxsd;	//keep track of maxsd
					}
				} else {
					// Old Client connection
					printf("\t\tSOY SERVER Y ESTOY RECIBIENDO DATA...\n");
					if ((bytes = recv(i, out, sizeof out, 0)) <= 0) { // got error or connection closed by client
						if (bytes == 0) // connection closed
							printf("selectserver: socket %d hung up\n", i);
						else
							perror("RECEIVE: Failed to receive from client.");
						close(i);
						FD_CLR(i, &master_set); // remove from master set
					} else {// we got some data from a client
						printf("\t\tSOY SERVER Y RECIBI DATA DE SD: %d - keyFrom: %d - keyTo: %d\n", i, out->keyFrom, out->keyTo);
						return out;
					}
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
		//close(clientSds[key]);
	}
	return out;

}

int sendMessage(NodeType to, Message * msg){
	int msglen;
	int i;

	if (to == CLIENT){	// I'm SERVER and have to send to CLIENT.
		unsigned int clientFromSize;
		int clientSd;
		struct sockaddr_in clientFrom;

		write_set = master_set; // Copy
		if ( select(maxsd + 1, NULL, &write_set, NULL, NULL) < 0 )
			errorLog("SEND: Failed to select socket.");

		for (i=0; i<maxsd+1; i++){
			if (FD_ISSET(i, &write_set)) {
				if (i == listener) {
                   			// New incomming connection
                    			clientFromSize = sizeof clientFrom;
                  			if ( (clientSd = accept(listener, (struct sockaddr *)&clientFrom, &clientFromSize)) == -1)
                        			errorLog("SEND: Failed to accept client connection.");
					else {
						FD_SET(clientSd, &master_set); // add to master set
						maxsd = (clientSd > maxsd)?clientSd:maxsd;	//keep track of maxsd
					}
				} else {
					// Old Client connection
					msglen = sizeof msg;
					printf("\t\tSOY SERVER Y ESTOY RECIBIENDO DATA\n");
					if (sendAll(i, msg, &msglen) < 0)
						errorLog("SEND: Failed to send to client.");
					else {
						printf("\t\tSOY SERVER Y MANDE DATA A SD: %d - keyFrom: %d - keyTo: %d\n", i, msg->keyFrom, msg->keyTo);
						return 0;
					}
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
		msglen = sizeof(msg);
		if (sendAll(clientSds[msg->keyFrom], msg, &msglen) < 0)
                	errorLog("Failed to send data to server.\n");	
		printf("************\nSOY CLIENT %d, MI CLIENTSD ES %d MANDE DATA...\n", msg->keyFrom, clientSds[msg->keyFrom]);
	//	close(clientSds[msg->keyFrom]);
	}
	return 0;
}

