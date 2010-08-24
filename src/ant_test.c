#include "../include/common.h"
#include "../include/communication.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>

int main(){

	signal(SIGINT, sigHandler);
	Message * smsg;
	Message * rmsg;

	printf("PID: %d \n", getpid());

	while( getchar() != EOF ){
		smsg = NULL;
		rmsg = NULL;

		Pos pos = { 0, 0 };
		smsg = createMessage(getpid(), -1, MOVE, SET, pos, 0);
		sendMessage(MAP, smsg);

		printf("Waiting to receive from server...\n");
		rmsg = receiveMessage(MAP);
		if( rmsg != NULL)
			printf("Message received from %d: %d \n", rmsg->pidFrom, rmsg->opCode);
	}
}
