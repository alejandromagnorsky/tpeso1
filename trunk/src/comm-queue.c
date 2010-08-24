#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#include "../include/communication.h"

static key_t mapKey = 1235;

typedef struct {
	long int type;
	Message msg;
} msgbuf;

Message * createMessage(int pidFrom,int pidTo, OpCode opCode, OpCodeParam param, Pos pos, double trace ){
	Message * out = malloc(sizeof(Message));
	out->pidTo = pidTo;
	out->pidFrom = pidFrom;
	out->opCode = opCode;
	out->param = param;	
	out->pos = pos;
	out->trace = trace;

	return out;
}


void sigHandler(){
	closeNode(0);
	exit(1);
}


void closeNode(NodeType t){
	int id;

	// Get descriptor
	if ( ( id = msgget(mapKey,O_WRONLY | O_CREAT |  0666)) == -1 )	
		return;
	msgctl(id, IPC_RMID , NULL); 
}



Message * receiveMessage(NodeType from){

	int id, prio;
	Message * out = NULL;
	msgbuf buf;


	if( from == MAP )
		prio = getpid(); // Ants receive (from Map) in queue getpid()
	else prio = 1; // Map receives in queue 1



	// Open
	if ( ( id = msgget(mapKey,O_RDONLY | O_CREAT | IPC_CREAT |0666)) == -1 )	{
		printf("error: %d \n", errno );
		return NULL;
	}

	// Receive
	if( msgrcv(id, &buf, sizeof(msgbuf), prio, 0) == -1 )
		return NULL;


	out = &(buf.msg);
	return out;
}

int sendMessage(NodeType to, Message * msg){

	int id;
	msgbuf buf;
	buf.msg = *msg;

	if( to == MAP )
		buf.type = 1; // Ants send to Map in queue 1
	else if( to == ANT )
		buf.type = msg->pidTo; // Map sends to ant by its pid
	else buf.type = 2; // Map sends to anthill in queue 2


	if ( ( id = msgget(mapKey,O_WRONLY | O_CREAT | IPC_CREAT | 0666)) == -1 )
		return -1;

	msgsnd(id, &buf, sizeof(msgbuf), 0);

	return 0;
}
