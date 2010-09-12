#include <stdio.h>
#include <stdlib.h>

#include "../../include/communication.h"
#include "../../include/transport.h"

Message * createMessage(int keyFrom,int keyTo, OpCode opCode, OpCodeParam param, Pos pos, double trace){
	Message * out = malloc(sizeof(Message));
	out->keyTo = keyTo;
	out->keyFrom = keyFrom;
	out->opCode = opCode;
	out->param = param;	
	out->pos = pos;
	out->trace = trace;

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
		default: break;
	}

	switch(msg->param){
		case SET: opCodeParamStr = "Set"; break;
		case GET: opCodeParamStr = "Get"; break;
		case OK: opCodeParamStr = "Ok"; break;
		case NOT_OK: opCodeParamStr = "Not Ok"; break;
		case OCCUPIED: opCodeParamStr = "Occupied"; break;
		case EMPTY: opCodeParamStr = "Empty"; break;
		case BIG: opCodeParamStr = "Big food"; break;
		default: break;
	}

	printf("OpCode: %s \n", opCodeStr);
	printf("Parameter: %s, Trace: %f, x=%d, y=%d \n", opCodeParamStr, msg->trace, msg->pos.x, msg->pos.y);
}



void closeServerIPC(){
	closeServer();
}

void closeClientIPC(){
	closeClient();
}

void openServerIPC(void * t){
	openServer(t, sizeof(Message));
}

void openClientIPC(void * t){
	openClient(t, sizeof(Message));
}

Message * receiveMessage(NodeType from, int key){
	Message * out = malloc(sizeof(Message));

	if( from == SERVER ){
		if(receiveFromServer( key, (char *)out, sizeof(Message) ) == -1 )
			errorLog("Failed to receive from server.");
	} else if( receiveFromClient(key,(char *)out,sizeof(Message)) == -1 )
			errorLog("Failed to receive from client.");

	return out;
}

int sendMessage(NodeType to, Message * msg){
	if( to == SERVER ){
		if(sendToServer(msg->keyTo,(char *)msg,sizeof(Message)) == -1 )
			errorLog("Failed to send to server.");
	}else if( sendToClient(msg->keyTo,(char *)msg,sizeof(Message)) == -1)
			errorLog("Failed to send to client.");
	return 0;
}

void
errorLog(char * s)
{
	perror(s);
	exit(1);
}

