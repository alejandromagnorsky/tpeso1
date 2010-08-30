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
#define MAP_PORT 6211

typedef struct {
	long int type;
	Message msg;
} msgbuf;

// TENGO QUE VER ESTO, PORQUE MI SIGHANDLER TIENE QUE RECIBER ANT, aANTHILL O MAP
void sigHandler(){
	closeNode(0);
	exit(1);
}


void closeNode(NodeType t){
	
}



Message * receiveMessage(NodeType from){
	int ssd, csd;	//Server/Client socket descriptors
	struct sockaddr_in server = { 0 }, client = { 0 };	//Server/Client structures
	Message * out = malloc(sizeof(Message));

	
	/* Server socket creation */
	if ((ssd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        	printf("Couldn't create server socket descriptor. Returning NULL.\n");
		return NULL;
        }

	memset(&server, 0, sizeof(server));	/* Zeroes struct */
	server.sin_family = AF_INET;		/* IP protocol */
	server.sin_port = htons(MAP_PORT);	/* Server port */
	if (from == MAP) {	// I'm CLIENT and I'm receiving data from SERVER.
		server.sin_addr.s_addr = htonl(LOCALHOST);	/* Local IP address */
		/* Connect to server */
printf("RECEIVE FROM MAP: CONNECT  |  pid: %d\n", getpid());
printf("RECEIVE: CONECTANDO CON SERVER: %s:%d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));
		if (connect(ssd, (struct sockaddr *) &server, sizeof(server)) < 0) {
             		printf("Failed to connect to the server. Returning NULL.\n");
			close(ssd);
			return NULL;
printf("RECEIVE FROM MAP: RECEIVE 1\n");
          	} else if (recv(ssd, out, sizeof(out), 0) < 1) {
                	printf("Failed to receive data from server. Returning NULL.\n");
			close(ssd);
			return NULL;
		}
	} else {	// I'm SERVER and I'm receiving data from CLIENT.
            	server.sin_addr.s_addr = INADDR_ANY;   /* Incoming addr */
		/* Binds server socket descriptor with server itself */
printf("RECEIVE FROM ANT: BIND\n");
		if (bind(ssd, (struct sockaddr *) &server, sizeof(server)) < 0) {
         		printf("Failed to bind the server socket. Returning NULL.\n");
			close(ssd);
			return NULL;
          	}
   		/* Server is willing to accept incoming connections */
printf("RECEIVE FROM ANT: LISTEN\n");
          	if (listen(ssd, 1) < 0) {	// 10 = Maximum pending connections accepted
          		printf("Failed to listen on server socket. Returning NULL.\n");
			close(ssd);
			return NULL;
          	}

		/* Server is ready to accept incoming connections. Server is a 'socket factory' */
		unsigned int clientSize = sizeof(client);
printf("RECEIVE FROM ANT: ACCEPT  |  pid: %d\n", getpid());
		if ((csd = accept(ssd, (struct sockaddr *) &client, &clientSize)) < 0) {
           	 	printf("Failed to accept client connection. Returning NULL.\n");
			close(ssd);
			close(csd);
			return NULL;
             	}
		/* Receiving data from client */
printf("RECEIVE: ATENDIENDO CLIENTE: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
printf("RECEIVE FROM ANT: RECV  |  pid: %d\n", getpid());
 		if (recv(csd, out, sizeof(out), 0) < 0) {
           		printf("Failed to receive from data client. Returning NULL.\n");
			close(ssd);
			close(csd);
			return NULL;
            	}		
	}
	close(csd);
	close(ssd);
	return out;
}

int sendMessage(NodeType to, Message * msg){
	int ssd, csd;	//Server/Client socket descriptors
	struct sockaddr_in server = { 0 }, client = { 0 };	//Server/Client structures
	
	/* Server socket creation */
	if ((ssd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        	printf("SEND: Couldn't create server socket descriptor. Returning -1.\n");
		return -1;
        }

	memset(&server, 0, sizeof(server));	/* Zeroes struct */
	server.sin_family = AF_INET;		/* IP protocol */
	server.sin_port = htons(MAP_PORT);	/* Server port */
	if (to == MAP) {	// I'm CLIENT and I'm receiving data from SERVER.
		server.sin_addr.s_addr = htonl(LOCALHOST);	/* Local IP address */
		/* Connect to server */
printf("SEND: CONECTANDOSE: %s:%d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));
printf("SEND: CONNECT\n");
		if (connect(ssd, (struct sockaddr *) &server, sizeof(server)) < 0) {
             		printf("SEND: Failed to connect to the server. Returning -1.\n");
			close(ssd);
			return -1;
printf("SEND: SEND 1 \n");
          	} else if (send(ssd, msg, sizeof(msg), 0) < 1) {
                	printf("SEND: Failed to send data to server. Returning -1.\n");
			close(ssd);
			return -1;
		}
	} else {	// I'm SERVER and I'm receiving data from CLIENT.
            	server.sin_addr.s_addr = INADDR_ANY;   /* Incoming addr */
		/* Binds server socket descriptor with server itself */
printf("SEND: BIND\n");
		if (bind(ssd, (struct sockaddr *) &server, sizeof(server)) < 0) {
         		printf("SEND: Failed to bind the server socket. Returning -1.\n");
			close(ssd);
			return -1;
          	}
   		/* Server is willing to accept incoming connections */
printf("SEND: LISTEN\n");
          	if (listen(ssd, 1) < 0) {	// 10 = Maximum pending connections accepted
          		printf("SEND: Failed to listen on server socket. Returning -1.\n");
			close(ssd);
			return -1;
          	}

		/* Server is ready to accept incoming connections. Server is a 'socket factory' */
printf("SEND: ACCEPT\n");
		unsigned int clientSize = sizeof(client);
		if ((csd = accept(ssd, (struct sockaddr *) &client, &clientSize)) < 0) {
           	 	printf("SEND: Failed to accept client connection. Returning -1.\n");
			close(ssd);
			close(csd);
			return -1;
             	}
		
		/* Sending data to client */
printf("SEND: ATENDIENDO CLIENTE: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
printf("SEND: SEND 2\n");
 		if (send(csd, msg, sizeof(msg), 0) < 0) {
           		printf("Failed to send data to client. Returning -1.\n");
			close(ssd);
			close(csd);
			return -1;
            	}
	}
	close(csd);
	close(ssd);
	return 0;
}
