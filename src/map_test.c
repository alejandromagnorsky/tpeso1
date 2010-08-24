#include "common.h"
#include "communication.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <mqueue.h>


int main(){

	int clients[10] = { 0 };

	Message * msg;

	// Wait till response of every ant (end of turn EOT)
	while(true){
		printf("Waiting to receive...\n");
		msg = receiveMessage(ANT);

		if( msg != NULL)
			printf("Message received from %d: %d \n", msg->pidFrom, msg->opCode);
	}
 }
