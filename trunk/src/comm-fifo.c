#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#include "../include/communication.h"

typedef struct {
	long int type;
	Message msg;
} msgbuf;

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

// TENGO QUE VER ESTO, PORQUE MI SIGHANDLER TIENE QUE RECIBER ANT, aANTHILL O MAP
void sigHandler(){
	closeNode(0);
	exit(1);
}


void closeNode(NodeType t){
	char * fifoName = NULL;

	if( t == MAP )
		sprintf(fifoName, "/tmp/fifo_map");
	else if( t == ANT )
		sprintf(fifoName, "/tmp/fifo_%d", getpid());
	else
		sprintf(fifoName, "/tmp/fifo_anthill");

	if ( unlink(fifoName) == -1 )
			printf("%s could not be unlinked", fifoName);
	return;
}



Message * receiveMessage(NodeType from){
	int fd;
	char * fifoName = NULL;
	Message * out = malloc(sizeof(Message));

	if( from == ANT )
		fifoName = "/tmp/fifo_map";
	else if( from == MAP ){
		// VER QUE ONDA ESTE MALLOC SI LO PUEDO SACAR DE ALGUN MANERA
		fifoName = malloc(16 * sizeof(char));
		sprintf(fifoName, "/tmp/fifo_%d", getpid());
	} else
		fifoName = "/tmp/fifo_anthill";

	if ( access(fifoName, 0) == -1 && mknod(fifoName, S_IFIFO | 0666, 0) == -1 ){
			printf("%s could not be created", fifoName);
			return NULL;
	}

	fd = open(fifoName, O_RDONLY);
	read(fd, out, sizeof(Message));

	close(fd);
	return out;
}

int sendMessage(NodeType to, Message * msg){
	int fd;
	char * fifoName = NULL;

	if( to == MAP )
		fifoName = "/tmp/fifo_map";
	else if( to == ANT ){
		// VER QUE ONDA ESTE MALLOC SI LO PUEDO SACAR DE ALGUN MANERA
		fifoName = malloc(14 * sizeof(char));
		sprintf(fifoName, "/tmp/fifo_%d", msg->pidTo);
	} else
		fifoName = "/tmp/fifo_anthill";

	if ( access(fifoName, 0) == -1 && mknod(fifoName, S_IFIFO | 0666, 0) == -1 ){
			printf("%s could not be created", fifoName);
			return -1;
	}

	fd = open(fifoName, O_WRONLY);
	write(fd, msg, sizeof(Message));

	close(fd);
	return 0;
}
