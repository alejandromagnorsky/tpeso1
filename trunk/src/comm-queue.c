#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#include "../include/communication.h"

static key_t SERVERKey = 1235;

static int queueID = 0;

typedef struct {
	long int type;
	Message msg;
} msgbuf;


void sigHandler(){
	destroyIPC();
	exit(1);
}

void destroyIPC(){
	msgctl(queueID, IPC_RMID, NULL); 
}

// Open & initialize IPC resource
void openIPC(){
	if ( ( queueID = msgget(SERVERKey,O_RDONLY | O_CREAT | IPC_CREAT |0666)) == -1 )
		errorLog("msgget");
	printf("HOLA: %d\n", queueID);
}

// Close IPC resource
void closeIPC(){

}


Message * receiveMessage(NodeType from){

	int prio;
	Message * out = NULL;
	msgbuf buf;


	if( from == SERVER )
		prio = getpid(); // CLIENTs receive (from SERVER) in queue getpid()
	else prio = 1; // SERVER receives in queue 1

	// Receive
	if( msgrcv(queueID, &buf, sizeof(msgbuf), prio, 0) == -1 )
		return NULL;

	// A copy must be made, because buf is deallocated after this function
	out = createMessage(buf.msg.keyFrom,buf.msg.keyTo, buf.msg.opCode,  buf.msg.param, buf.msg.pos,buf.msg.trace);
	out->fromPos.x = buf.msg.fromPos.x;
	out->fromPos.y = buf.msg.fromPos.y;
	return out;
}

int sendMessage(NodeType to, Message * msg){

	msgbuf buf;
	buf.msg = *msg;

	if( to == SERVER )
		buf.type = 1; // CLIENTs send to SERVER in queue 1
	else if( to == CLIENT )
		buf.type = msg->keyTo; // SERVER sends to CLIENT by its pid

	msgsnd(queueID, &buf, sizeof(msgbuf), 0);

	return 0;
}
