#include <stdio.h>
#include <stdlib.h>

#include "../include/common.h"


Message * createMessage(int keyFrom,int keyTo, OpCode opCode, OpCodeParam param, Pos pos, double trace){
	Message * out = malloc(sizeof(Message));
	out->keyTo = keyTo;
	out->keyFrom = keyFrom;
	out->opCode = opCode;
	out->param = param;	
	out->pos = pos;
	out->trace = trace;

	// FRONTEND ONLY
	Pos fromPos = {0,0};
	out->fromPos = fromPos;

	return out;
}


void printMessage(Message * msg){
	printf("Message Data || ");
	printf("From: %d To: %d  || ", msg->keyFrom, msg->keyTo);

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
		case EMPTY: opCodeParamStr = "Empty"; break;
		case BIG: opCodeParamStr = "Big food"; break;
	}

	printf("OpCode: %s \n", opCodeStr);
	printf("Parameter: %s, Trace: %f, x=%d, y=%d \n", opCodeParamStr, msg->trace, msg->pos.x, msg->pos.y);
	printf("From: (%d,%d)\n", msg->fromPos.x, msg->fromPos.y);
}

void
errorLog(char * s)
{
	perror(s);
	exit(1);
}

