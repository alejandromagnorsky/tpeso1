#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define SERVER_IP INADDR_LOOPBACK
#define SERVER_PORT 6055

void die(char * msg){
	printf("SERVER: %s\n", msg);
	exit(1);
}

int main(){
	int ssd, csd, clientSize, msgReceived;
	char buffer[250];
	struct sockaddr_in server, client;

	if ((ssd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        	die("Failed to create socket");

	memset(&server, 0, sizeof(server));     /* Zero out structure */
   	server.sin_family      = AF_INET;             /* Internet address family */
  	server.sin_addr.s_addr = INADDR_ANY;   /* Server IP address */
    	server.sin_port        = htons(SERVER_PORT); /* Server port */

	/* Bind to the local address */
	if (bind(ssd, (struct sockaddr *) &server, sizeof(server)) < 0)
        	die("Failed to bind server");

	/* Mark the socket so it will listen for incoming connections */
	if (listen(ssd, 1) < 0)
		die("Failed to listen to socket");
 
	for (;;) /* Run forever */
  	{
        	/* Set the size of the in-out parameter */
        	clientSize = sizeof(client);

        	/* Wait for a client to connect */
        	if ((csd = accept(ssd, (struct sockaddr *) &client, &clientSize)) < 0)
        		die("Failed to accept incoming connection");
	
        	/* clntSock is connected to a client! */
        	printf("ATENDIENDO CLIENTE: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        	/* Receive message from client */
    		if ( (msgReceived = recv(csd, buffer, sizeof buffer, 0)) < 0)
        		die("Failed to receive data from server");
		
		buffer[msgReceived] = '\0';
		printf("RECIBIDO: %s\n", buffer);
   	}



}

