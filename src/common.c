#include <stdio.h>
#include <stdlib.h>

#include "../include/common.h"


Message * createMessage(int pidFrom,int pidTo, OpCode opCode, OpCodeParam param, Pos pos, double trace){
	Message * out = malloc(sizeof(Message));
	out->pidTo = pidTo;
	out->pidFrom = pidFrom;
	out->opCode = opCode;
	out->param = param;	
	out->pos = pos;
	out->trace = trace;

	return out;
}


void printMessage(Message * msg){
	printf("Message Data || ");
	printf("From: %d To: %d  || ", msg->pidFrom, msg->pidTo);

	char * opCodeStr = "Invalid";
	char * opCodeParamStr = "Invalid";

	switch(msg->opCode){
		case REGISTER: opCodeStr = "Register"; break;
		case MOVE: opCodeStr = "Move"; break;
		case FOOD: opCodeStr = "Food"; break;
		case SHOUT: opCodeStr = "SHOUT"; break;
		case TRACE: opCodeStr = "Trace"; break;
		case TURN: opCodeStr = "Turn"; break;
	}

	switch(msg->param){
		case SET: opCodeParamStr = "Set"; break;
		case GET: opCodeParamStr = "Get"; break;
		case OK: opCodeParamStr = "Ok"; break;
		case NOT_OK: opCodeParamStr = "Not Ok"; break;
		case OCCUPIED: opCodeParamStr = "Occupied"; break;
		case BIG: opCodeParamStr = "Big food"; break;
	}

	printf("OpCode: %s \n", opCodeStr);
	printf("Parameter: %s, Trace: %f, x=%d, y=%d \n", opCodeParamStr, msg->trace, msg->pos.x, msg->pos.y);
}

