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
#define SERVER_PORT 8888

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
	int csd;
	unsigned int clientFromSize;
	struct sockaddr_in clientFrom;
	char * str = "I'm SERVER and I command you to receive my data.";
	Message * out = malloc(sizeof (Message));

	// I'm always SERVER and receive data from CLIENT to send it back *** the same data? ***
	clientFromSize = sizeof clientFrom;
	if ( (csd = accept(serverSd, (struct sockaddr *) &clientFrom, &clientFromSize)) < 0)
		errorLog("SERVER: Failed to accept connection.");

	if ( recv(csd, out, sizeof out, 0) < 0)
		errorLog("SERVER: Failed to receive data from client.");
	
	if ( send(csd, str, sizeof str, 0) < 0)
		errorLog("SERVER: Failed to send data to client.");

	return out;

}

int sendMessage(NodeType to, Message * msg){
	int ssd;
	char * str = malloc(250);

	if ((ssd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		errorLog("CLIENT: Failed to create socket.");
	if ((connect(ssd, (struct sockaddr *) &clientSide, sizeof clientSide)) < 0)
		errorLog("CLIENT: Failed to connect to server socket.");

	if (send(ssd, msg, sizeof(msg), 0) < 0)
                errorLog("CLIENT: Failed to send data to server.\n");
	if (recv(ssd, str, sizeof(str), 0) < 0)
                errorLog("CLIENT: Failed to receive data from server.\n");

	return 0;
}

