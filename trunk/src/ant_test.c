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
#include <pthread.h>


void testSETSHOUT(){
	Pos pos = {0,0};

	Message * send;	
	Message * received;

	printf("I want to SHOUT! \n");
	send = createMessage( getpid(), -1, SHOUT, SET, pos, 0);
	printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == SHOUT && received->param == OK )
		printf("I shout!\n");
	else if(received->opCode == TURN && received->param == NOT_OK)
		printf("I don't have turns left! \n");
}

void testSETFood(int x, int y){
	Pos pos = {x,y};

	Message * send;	
	Message * received;

	printf("I want to leave food on anthill at (%d,%d) from here \n", x,y);
	send = createMessage( getpid(), -1, FOOD, SET, pos, 0);
	printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == FOOD && received->param == OK )
		printf("I left food at anthill!\n");
	else if(received->opCode == FOOD && received->param == EMPTY)
		printf("I have no food! \n");
	else printf("I tried to leave food on floor! \n");
}

void testGETFood(int x, int y){
	Pos pos = {x,y};

	Message * send;	
	Message * received;

	printf("I want get food at (%d,%d) from here\n", x,y);
	send = createMessage( getpid(), -1, FOOD, GET, pos, 0);
	printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == FOOD && received->param == OK )
		printf("I got food! \n");
	else if(received->opCode == FOOD && received->param == BIG)
		printf("I cannot get food, too big! \n");
	else printf("I cannot get food =( \n");
}



void testGETMove(int x, int y){
	Pos pos = {x,y};

	Message * send;	
	Message * received;

	printf("I want check to move at (%d,%d) from here\n", x,y);
	send = createMessage( getpid(), -1, MOVE, GET, pos, 0);
	printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == MOVE && received->param == EMPTY )
		printf("I can move! \n");
	else if(received->opCode == FOOD && received->param == OCCUPIED)
		printf("I cannot move! There is FOOD! \n");
	else printf("I cannot move! =( \n");
}


void testSETMove(int x, int y){
	Pos pos = {x,y};

	Message * send;	
	Message * received;

	printf("I want to move at (%d,%d) from here \n", x,y);
	send = createMessage( getpid(), -1, MOVE, SET, pos, 0);
	printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == MOVE && received->param == OK )
		printf("I moved! \n");
	else printf("Couldn't move! =( \n");
}


void testGETRegister(){

	Message * send;
	Message * received;

	Pos pos = {0,0};

	printf("Am I registered? \n");
	send = createMessage( getpid(), -1, REGISTER, GET, pos, 0);
	printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == REGISTER && received->param == OK )
		printf("Registered. \n");
	else printf("Not registered. \n");

}

void testSETRegister(){

	Message * send;
	Message * received;

	Pos pos = {0,0};

	// Sending first register
	printf("Trying to register... \n");
	send = createMessage( getpid(), -1, REGISTER, SET, pos, 0);
	printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == REGISTER && received->param == OK )
		printf("Register successful. Anthillpos: (%d,%d)\n",received->pos.x, received->pos.y);
	else printf("Register failed.\n");

}


int main(){
	printf("PID: %d \n", getpid());


	openIPC();

	/* 
	La idea:


	while(1){
		receive from Map TURN SET
	
		do_things

		Message * msgR;
		msgR = receiveMessage(SERVER);

		printf("Received from map: %d \n", msgR->opCode);
	}
	*/
	
	


	//REGISTER GET/SET -----------------------------

	// Must succeed
	getchar();
	testGETRegister();

	// Must succeed
	getchar();
	testSETRegister();

	// Must fail
	getchar();
	testSETRegister();

	// Must fail
	getchar();
	testGETRegister();

	// ORIGINAL SEQUENCE MUST WORK AS STATED
	// However, if several ant_tests are called
	// the map will mantain them (even if they are dead)
	// so the sequence will not be the same
	// although it works as it should be.

	// MOVE SET-----------------------------

	// Must succeed, toy en 4,5
	getchar();
	testSETMove(1,0);

	// Must FAIL
	getchar();
	testSETMove(0,0);

	// Must FAIL
	getchar();
	testSETMove(10,10);

	// Must succeed, toy en 5,5
	getchar();
	testSETMove(1,0);

	// Must succeed, toy en 6,5
	getchar();
	testSETMove(1,0);

	// Must fail, food is in 7,5
	getchar();
	testSETMove(1,0);

	// MOVE GET -----------------------------

	// Must FAIL, food there
	getchar();
	testGETMove(1,0);

	// Must work
	getchar();
	testGETMove(0,1);

	// Must fail
	getchar();
	testGETMove(-2,-1);

	// FOOD GET ------------------------------

	// Must fail
	getchar();
	testGETFood(0,0);

	// Must fail
	getchar();
	testGETFood(2,3);
	
	// Must fail
	getchar();
	testGETFood(-2,0);

	// Must work
	getchar();
	testGETFood(1,0);

	// Must fail, already got food
	getchar();
	testGETFood(1,0);

	// FOOD SET ----------------------

	// Must succeed, toy en 5,5
	getchar();
	testSETMove(-1,0);

	// Must succeed, toy en 4,5
	getchar();
	testSETMove(-1,0);

	// Must fail
	getchar();
	testSETFood(1,0);

	// Must succeed
	getchar();
	testSETFood(-1,0);

	// Must fail
	getchar();
	testSETFood(0,0);

	// SHOUT -------------------------------------

	// Must succeed
	getchar();
	testSETSHOUT();

	closeIPC();

}
