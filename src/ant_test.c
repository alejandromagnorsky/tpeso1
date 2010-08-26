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



void printMessage(Message * msg){

	printf("\nMessage Data \n");
	printf("From: %d To: %d \n", msg->pidFrom, msg->pidTo);

	char * opCodeStr = "Invalid";
	char * opCodeParamStr = "Invalid";

	switch(msg->opCode){
		case REGISTER: opCodeStr = "Register"; break;
		case NEIGHBOR: opCodeStr = "Neighbor"; break;
		case MOVE: opCodeStr = "Move"; break;
		case FOOD: opCodeStr = "Food"; break;
		case SHOUT: opCodeStr = "SHOUT"; break;
		case TRACE: opCodeStr = "Trace"; break;
		case RECEIVED: opCodeStr = "Received"; break;
		case TURN: opCodeStr = "Turn"; break;
	}

	switch(msg->param){
		case SET: opCodeParamStr = "Set"; break;
		case GET: opCodeParamStr = "Get"; break;
		case OK: opCodeParamStr = "Ok"; break;
		case NOT_OK: opCodeParamStr = "Not Ok"; break;
	}

	printf("OpCode: %s \n", opCodeStr);
	printf("Parameter: %s, Trace: %f, x=%d, y=%d \n", opCodeParamStr, msg->trace, msg->pos.x, msg->pos.y);
}


int main(){

	signal(SIGINT, sigHandler);
	Message * smsg;
	Message * rmsg;

	printf("PID: %d \n", getpid());

	Pos pos = { 0, 0 };
	while( getchar() != EOF ){
		smsg = NULL;
		rmsg = NULL;

		smsg = createMessage(getpid(), -1, REGISTER, GET, pos, 0);
		sendMessage(MAP, smsg);
		
		printf("Waiting to receive from server...\n");
		rmsg = receiveMessage(MAP);
		if( rmsg != NULL) {
			printf("Message received. \n");
			printMessage(rmsg);
		}


		smsg = createMessage(getpid(), -1, REGISTER, SET, pos, 0);
		sendMessage(MAP, smsg);
		
		printf("Waiting to receive from server...\n");
		rmsg = receiveMessage(MAP);
		if( rmsg != NULL) {
			printf("Message received. \n");
			printMessage(rmsg);
		}


		smsg = createMessage(getpid(), -1, REGISTER, GET, pos, 0);
		sendMessage(MAP, smsg);
		
		printf("Waiting to receive from server...\n");
		rmsg = receiveMessage(MAP);
		if( rmsg != NULL) {
			printf("Message received. \n");
			printMessage(rmsg);
		}
		
	}
}
