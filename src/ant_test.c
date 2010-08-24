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

	while( getchar() != EOF ){
		Pos pos = { 0, 0 };
		Message * msg = createMessage(-1,getpid(), REGISTER, SET, pos, 0 );
		sendMessage(MAP, msg);
		printf("Sent data: %d \n", msg->opCode);
	}
}
