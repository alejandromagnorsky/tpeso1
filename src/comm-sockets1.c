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

// Socket descriptors
int serverSd;
int serverSds;
int clientSds[CLIENTQUANT];
int clientUseFlags[CLIENTQUANT];
int serverUseFlag;

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

	/* Initialize vectors with -1 */
	int i;
	for(i=0; i<CLIENTQUANT; i++){
	//	serverSds[i] = -1;
		clientSds[i] = -1;
		clientUseFlags[i] = 2;
	}
	serverUseFlag = 2;

	/* Fill serverSide structure */
	memset(&serverSide, 0, sizeof(struct sockaddr_in));	/* Zeroes struct */
	serverSide.sin_family = AF_INET;		/* IP protocol */
	serverSide.sin_addr.s_addr = INADDR_ANY;  	/* Server IP address */
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
}

// Close IPC resource
void closeIPC(){
	if ( close(serverSd) == -1 )
		errorLog("Server's socket descriptor could not be closed.\n");
}



Message * receiveMessage(NodeType from, int key){
	unsigned int clientFromSize;
	struct sockaddr_in clientFrom;
	Message * out = malloc(sizeof(Message));

	
	if (from == SERVER) {	// I'm CLIENT and I'm receiving data from SERVER.







		if(clientUseFlags[key] == 2){
			if ((clientSds[key] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		   		errorLog("Failed to create client socket descriptor.");

			if (connect(clientSds[key], (struct sockaddr *) &clientSide, sizeof(clientSide)) < 0)
	             		errorLog("Failed to connect client socket to the server.");			
		}

		printf("SOY CLIENTE clientSds(%d): %d, RECIBIENDO DATA... \n", key, clientSds[key]);
          	if (recv(clientSds[key], out, sizeof(out), 0) < 1)
                	errorLog("Failed to receve data from server.\n");
		printf("SOY CLIENTE clientSds(%d): %d, RECIBI DATA \n", key, clientSds[key]);
	
		if (--clientUseFlags[key] == 0){
			close(clientSds[key]);
			clientUseFlags[key] = 2;
		}







	} else {		// I'm SERVER and I'm receiving data from CLIENT.
		

		if(serverUseFlag == 2){
			if ((serverSds = accept(serverSd, (struct sockaddr *) &clientFrom, &clientFromSize)) < 0)
     	      	 		errorLog("SEND: Failed to accept client connection.\n");	
		}

		printf("SOY SERVER RECIBIENDO DE serverSds(%d): %d ....\n", key, serverSds);
		if (recv(serverSds, out, sizeof(out), 0) < 0)
	       		errorLog("Failed to receive data from client.");
		printf("SOY SERVER RECIBI DATA DE serverSds(%d): %d \n", key, serverSds);
		
		if (--serverUseFlag == 0){
			close(serverSds);
			serverUseFlag = 2;
		}



		
	}
	return out;
}

int sendMessage(NodeType to, Message * msg){
	//int sd;	//Server/Client socket descriptors
	unsigned int clientFromSize;
	struct sockaddr_in clientFrom;

	if (to == SERVER) {	// I'm CLIENT and I'm sending data to SERVER.






		if(clientUseFlags[msg->keyFrom] == 2){
			if ((clientSds[msg->keyFrom] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		   		errorLog("Failed to create client socket descriptor.");

			if (connect(clientSds[msg->keyFrom], (struct sockaddr *) &clientSide, sizeof(clientSide)) < 0)
	             		errorLog("Failed to connect client socket to the server.");			
		}

		printf("SOY CLIENTE clientSds(%d): %d, MANDANDO DATA... \n", msg->keyFrom, clientSds[msg->keyFrom]);
          	if (send(clientSds[msg->keyFrom], msg, sizeof(msg), 0) < 1)
                	errorLog("Failed to send data to server.\n");
		printf("SOY CLIENTE clientSds(%d): %d, MANDÉ DATA \n", msg->keyFrom, clientSds[msg->keyFrom]);
	
		if (--clientUseFlags[msg->keyFrom] == 0){
			close(clientSds[msg->keyFrom]);
			clientUseFlags[msg->keyFrom] = 2;
		}







	} else {	// I'm SERVER and I'm sending data to CLIENT.





		if(serverUseFlag == 2){
			if ((serverSds = accept(serverSd, (struct sockaddr *) &clientFrom, &clientFromSize)) < 0)
     	      	 		errorLog("Failed to accept client connection.\n");	
		}

		
		printf("SOY SERVER MANDANDO A serverSds(%d): %d ....\n", msg->keyTo, serverSds);
		if (send(serverSds, msg, sizeof(msg), 0) < 0)
	       		errorLog("SEND: Failed to send data to client.\n");
		printf("SOY SERVER MANDE DATA A serverSds(%d): %d \n", msg->keyTo, serverSds);
		
		if (--serverUseFlag == 0){
			close(serverSds);
			serverUseFlag = 2;
		}



	}
	return 0;
}

