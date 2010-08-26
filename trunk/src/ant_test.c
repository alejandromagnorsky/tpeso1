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
	sendMessage(MAP, send);

	received = receiveMessage(MAP);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == SHOUT && received->param == OK )
		printf("I shout!\n");
	else if(received->opCode == TURN && received->param == NOT_OK)
		printf("I don't have turns left! \n");
}



void testSETTrace(double trace){
	Pos pos = {0,0};

	Message * send;	
	Message * received;

	printf("I want to leave trace here \n");
	send = createMessage( getpid(), -1, TRACE, SET, pos, trace);
	printMessage(send);
	sendMessage(MAP, send);

	received = receiveMessage(MAP);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == TRACE && received->param == OK )
		printf("I left trace!\n");
	else if(received->opCode == TRACE && received->param == NOT_OK)
		printf("I tried to leave an invalid trace! \n");
}


void testSETFood(int x, int y){
	Pos pos = {x,y};

	Message * send;	
	Message * received;

	printf("I want to leave food on anthill in (%d,%d) \n", x,y);
	send = createMessage( getpid(), -1, FOOD, SET, pos, 0);
	printMessage(send);
	sendMessage(MAP, send);

	received = receiveMessage(MAP);
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

	printf("I want get food from (%d,%d) \n", x,y);
	send = createMessage( getpid(), -1, FOOD, GET, pos, 0);
	printMessage(send);
	sendMessage(MAP, send);

	received = receiveMessage(MAP);
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

	printf("I want check to move to (%d,%d) \n", x,y);
	send = createMessage( getpid(), -1, MOVE, GET, pos, 0);
	printMessage(send);
	sendMessage(MAP, send);

	received = receiveMessage(MAP);
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

	printf("I want to move to (%d,%d) \n", x,y);
	send = createMessage( getpid(), -1, MOVE, SET, pos, 0);
	printMessage(send);
	sendMessage(MAP, send);

	received = receiveMessage(MAP);
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
	sendMessage(MAP, send);

	received = receiveMessage(MAP);
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

	/* 
	La idea:


	while(1){
		receive from Map TURN SET
	
		do_things
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

	// Must succeed
	getchar();
	testSETMove(4,5);

	// Must FAIL
	getchar();
	testSETMove(3,5);

	// Must FAIL
	getchar();
	testSETMove(14,15);

	// Must succeed
	getchar();
	testSETMove(4,5);

	// Must succeed
	getchar();
	testSETMove(5,5);

	// Must succeed
	getchar();
	testSETMove(6,5);

	// MOVE GET -----------------------------

	// Must FAIL
	getchar();
	testGETMove(6,5);

	// Must work
	getchar();
	testGETMove(6,6);

	// Must fail
	getchar();
	testGETMove(4,5);

	// Must fail
	getchar();
	testGETMove(5,4);

	// Must fail, FOOD is there
	getchar();
	testGETMove(7,5);

	// FOOD GET ------------------------------

	// Must fail
	getchar();
	testGETFood(6,5);

	// Must fail
	getchar();
	testGETFood(2,3);
	
	// Must fail
	getchar();
	testGETFood(5,5);

	// Must work
	getchar();
	testGETFood(7,5);

	// Must fail, already got food
	getchar();
	testGETFood(7,5);

	// FOOD SET ----------------------

	// Must succeed
	getchar();
	testSETMove(5,5);

	// Must succeed
	getchar();
	testSETMove(4,5);

	// Must fail
	getchar();
	testSETFood(5,5);

	// Must succeed
	getchar();
	testSETFood(3,5);

	// Must fail
	getchar();
	testSETFood(3,5);

	// SHOUT -------------------------------------

	// Must succeed
	getchar();
	testSETSHOUT();

}
