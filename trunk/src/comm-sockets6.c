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
#include "../include/communication.h"

#define LOCALHOST INADDR_LOOPBACK
#define SERVER_IP LOCALHOST
#define SERVER_PORT 8888

int clientCount = 0;

#define INVALID_SOCKET -1

// Socket descriptors
int serverSd;
int * clientSds; //[clientCount];
int * acceptedClients;//[clientCount];

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

void * acceptClients(void * t){

	int i;
	for (i=3; i<clientCount; i++){
	
	//	printf("Esperando nuevas conexiones... %d\n",i);

		if ((acceptedClients[i] = accept(serverSd, NULL, NULL)) < 0)
			errorLog("Failed to accept connection from client.");

		toRead[i].fd = acceptedClients[i];
		toRead[i].events = POLLIN;
		toRead[i].revents = 0;

	//	printf("Nueva conexion aceptada!\n");
	}

	printf("Muere el thread.\n");

	pthread_exit(NULL);
}


// Open & initialize IPC resource
void openServer(void *t){
	clientCount = (int) t + 3;

	int optionValue = 1;

	/* Fill serverSide structure */
	memset(&serverSide, 0, sizeof(struct sockaddr_in));	/* Zeroes struct */
	serverSide.sin_family = AF_INET;			/* IP protocol */
	serverSide.sin_addr.s_addr = INADDR_ANY;  		/* Server IP address */
	serverSide.sin_port = htons(SERVER_PORT);		/* Server port */

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
	if (listen(serverSd, clientCount) < 0)
		errorLog("Failed to listen to socket");

	toRead = malloc(clientCount * sizeof(struct pollfd));
	acceptedClients = malloc(clientCount * sizeof(int));

	int i;
	for (i=0; i<clientCount; i++){
		toRead[i].fd = INVALID_SOCKET;
	}

	printf("Termino el server: llamo a nuevo thread.\n");

	pthread_t acceptClientThread;
	pthread_create(&acceptClientThread, NULL, acceptClients, NULL);

	printf("Nuevo thread creado.\n");
}

void openClient(void *t){
	clientCount = (int) t + 3;

	/* Fill clientSide structure */
	memset(&clientSide, 0, sizeof(struct sockaddr_in));
	clientSide.sin_family = AF_INET;
	clientSide.sin_addr.s_addr = htonl(SERVER_IP);
	clientSide.sin_port = htons(SERVER_PORT);

	int i;
	clientSds = malloc(clientCount * sizeof(int));

	//int flags;
	for (i=3; i<clientCount; i++){
		if ((clientSds[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		       	errorLog("Could not create client socket.");

		if (connect(clientSds[i], (struct sockaddr *) &clientSide, sizeof(clientSide)) < 0)
        		errorLog("Failed to connect to server.");
	}

	printf("Termino de conectar clientes.\n");
}

// Close IPC resource
void closeIPC(){
	if ( close(serverSd) == -1 )
		errorLog("Server's socket descriptor could not be closed.\n");
}



Message * receiveMessage(NodeType from, int key){
//	printf("SOY: %s Y ENTRE A RECEIVE *******************\n", (from == CLIENT)?"SERVER":"CLIENT");
	Message * out = malloc(sizeof (Message));

	int sd, i, polled;
	if (from == CLIENT){	// I'm SERVER and I receive from CLIENT.
		polled = poll(toRead, clientCount, 5000000);
	//	printf("POLL: %d\n", polled);

		if (polled == 0)
			errorLog("poll() timed out.");
		else if (polled < 0)
			errorLog("Failed to poll()");



		for (i = 3; i<clientCount; i++){
			if (toRead[i].revents & POLLIN){
				sd = toRead[i].fd;
				toRead[i].revents = 0;
				break;
			}
		}

	//	printf("Server recibe data de %d\n", sd);

		void * tmp = malloc(sizeof(Message));
		
		if ( recv(sd, tmp,sizeof(Message), 0) < 0)
			errorLog("SERVER: Failed to receive data from client.");

		out = (Message*)tmp;
	//	printf("SOY SERVER RECIBI DATA\n");
	//	printMessage((Message*)tmp);

	} else {		// I'm CLIENT and I receive from SERVER.
		if ( recv(clientSds[key], out, sizeof(Message), 0) < 0)
			errorLog("CLIENT: Failed to receive data from server.");
	}



	//printf("SOY %s Y SALI DE RECEIVE *******************\n", (from == CLIENT)?"SERVER":"CLIENT");
	return out;
}

int sendMessage(NodeType to, Message * msg){
//	printf("SOY: %s Y ENTRE A SEND ******************\n", (to == CLIENT)?"SERVER":"CLIENT");
	//int i;
	//for (i=0; i<clientCount; i++){
		//printf("SEND:\tclientSds(%d): %d  -  acceptedClients(%d): %d  -  toRead(%d).fd: %d\n", i, clientSds[i], i, acceptedClients[i], i, toRead[i].fd);
	//}

	//int a = clientSds[msg->keyFrom];
	//printf("clientSds %d\n", a);

	if (to == CLIENT){	// I'm SERVER and I send to CLIENT.	
		if ( send(acceptedClients[msg->keyTo], msg, sizeof(Message), 0) < 0)
			errorLog("SERVER: Failed to send data to client.");
	} else {		// I'm CLIENT and I send to SERVER.
	//	printf("Cliente %d con FD %d envia\n", msg->keyFrom, clientSds[msg->keyFrom]);
		if ( send(clientSds[msg->keyFrom], msg, sizeof(Message), 0) < 0)
			errorLog("CLIENT: Failed to send data to server.");
	//	printf("I AM CLIENT %d AND I SENT TO SERVER %d\n", msg->keyFrom, msg->keyTo);
	}

	//printf("SOY %s Y SALI DE SEND *******************\n", (to == CLIENT)?"SERVER":"CLIENT");
	return 0;
}
