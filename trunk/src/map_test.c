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

	while(true){

		smsg = NULL;
		rmsg = NULL;

		printf("Waiting to receive...\n");
		rmsg = receiveMessage(ANT);
		int ant_id = rmsg->pidFrom;

		if( rmsg != NULL){
			printf("Message received from %d: %d \n", ant_id, rmsg->opCode);
			Pos pos = { 0, 0 };
			smsg = createMessage(getpid(), ant_id, RECEIVED, OK, pos, 0 );
			sendMessage(ANT, smsg);
		}
	}
 }
