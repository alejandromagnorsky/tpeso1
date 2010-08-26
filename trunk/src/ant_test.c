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

void testRegister(int x, int y){

	Message * send;
	Message * received;

	Pos pos = {x,y};

	// Sending first register
	printf("Trying to register... \n");
	send = createMessage( getpid(), -1, REGISTER, SET, pos, 0);
	printMessage(send);
	sendMessage(MAP, send);

	received = receiveMessage(MAP);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == REGISTER && received->param == OK )
		printf("Register successful.\n");
	else printf("Register failed.\n");

}


int main(){
	printf("PID: %d \n", getpid());

	// Must succeed
	getchar();
	testRegister(0,0);

	// Must fail
	getchar();
	testRegister(0,0);

	// Must succeed
	getchar();
	testRegister(1,0);
}
