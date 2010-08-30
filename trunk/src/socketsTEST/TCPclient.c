#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define SERVER_IP INADDR_LOOPBACK
#define SERVER_PORT 6055

void die(char * msg){
	printf("CLIENT: %s\n", msg);
	exit(1);
}

int main(int argc, char * argv[]){
	int ssd;
	char * str = argv[1];
	struct sockaddr_in server;

	/* Create socket for incoming connections */
	if ((ssd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        	die("Failed to create socket");

	memset(&server, 0, sizeof(server));     /* Zero out structure */
   	server.sin_family      = AF_INET;             /* Internet address family */
  	server.sin_addr.s_addr = htonl(SERVER_IP);   /* Server IP address */
    	server.sin_port        = htons(SERVER_PORT); /* Server port */

	/* Establish the connection to the echo server */
	if (connect(ssd, (struct sockaddr *) &server, sizeof(server)) < 0)
        	die("Failed to connect to server");

	/* Send the string to the server */
	if (send(ssd, str, strlen(str), 0) != strlen(str))
		die("Sending data error");
	
	printf("END INTERACTION\n");
	close(ssd);
	return 0;


}
