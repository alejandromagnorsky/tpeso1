#include "../include/common.h"
#include "../include/communication.h"
#include "../include/map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>

#define SIZE_X 50
#define SIZE_Y 50


void printMessage(Message * msg){

	printf("\nMessage Data \n");
	printf("From: %d To: %d \n", msg->pidFrom, msg->pidTo);

	char * opCodeStr = "Invalid";
	char * opCodeParamStr = "Invalid";

	switch(msg->opCode){
		case REGISTER: opCodeStr = "Register"; break;
		case NEIGHBOR: opCodeStr = "Neighbor"; break;
		case MOVE: opCodeStr = "Move"; break;
		case FOOD: opCodeStr = "Food"; break;
		case SHOUT: opCodeStr = "SHOUT"; break;
		case TRACE: opCodeStr = "Trace"; break;
		case RECEIVED: opCodeStr = "Received"; break;
		case TURN: opCodeStr = "Turn"; break;
	}

	switch(msg->param){
		case SET: opCodeParamStr = "Set"; break;
		case GET: opCodeParamStr = "Get"; break;
		case OK: opCodeParamStr = "Ok"; break;
		case NOT_OK: opCodeParamStr = "Not Ok"; break;
	}

	printf("OpCode: %s \n", opCodeStr);
	printf("Parameter: %s, Trace: %f, x=%d, y=%d \n", opCodeParamStr, msg->trace, msg->pos.x, msg->pos.y);
}


bool exists(int pid,World * world){
	int i;
	for(i=0;i<world->maxConnections;i++)
		if(world->clients[i] == pid)
			return true;
	return false;
}

bool isOccupied(Pos * pos, World * world){
	return world->cells[pos->x][pos->y].type != EMPTY_CELL;
}

void registerAnt(Message * msg, World * world){

	Message * ans;
	int x,y,i;
	x = msg->pos.x;
	y = msg->pos.y;

	ans = createMessage( getpid(), msg->pidFrom, REGISTER, NOT_OK, msg->pos, msg->trace );

	if(!exists(msg->pidFrom, world) && !isOccupied(&msg->pos, world)){
		// Get next empty ant slot
		for(i=0;i<world->maxConnections && world->clients[i] != INVALID_ID;i++);

		// Register ant, position it on map
		if( i != world->maxConnections && world->clients[i] == INVALID_ID ){
			world->clients[i] = msg->pidFrom;
			Cell cell;
			cell.trace = world->cells[x][y].trace; // trace is mantained
			cell.foodType = NO_FOOD; // cannot register on food
			cell.typeID = msg->pidFrom;
			world->cells[msg->pos.x][msg->pos.y] = cell;
			ans = createMessage( getpid(), msg->pidFrom, REGISTER, OK, msg->pos, world->cells[x][y].trace);
		}
	}

	sendMessage(ANT, ans);
}

void checkRegistered(Message * msg, World * world){
	int i;
	Message * ans;
	ans = createMessage( getpid(), msg->pidFrom, REGISTER, NOT_OK, msg->pos, msg->trace);
	for(i=0;i<world->maxConnections;i++)
		if(world->clients[i] == msg->pidFrom)
			ans = createMessage( getpid(), msg->pidFrom, REGISTER, OK, msg->pos, msg->trace);

	sendMessage(ANT, ans);
}

void parseMessage(Message * msg, World * world){

	// Parse opCode
	switch(msg->opCode){
		case REGISTER:
			if(msg->param == SET)
				registerAnt(msg, world);
			if(msg->param == GET)
				checkRegistered(msg, world);
			break;
	}
}

World * getWorld( int sizeX, int sizeY, int maxConnections, int turnsLeft){

	int i,j;
	World * out = malloc(sizeof(World));

	out->sizeX = sizeX;
	out->sizeY = sizeY;
	out->turnsLeft = turnsLeft;
	out->maxConnections = maxConnections;


	// Allocation of collections
	out->clients = malloc(maxConnections * sizeof(int));
	for(i=0;i<maxConnections;i++)
		out->clients[i] = INVALID_ID;

	out->cells = malloc(sizeX * sizeof(Cell *));
	for(i=0;i<sizeX;i++)
		out->cells[i] = malloc(sizeY * sizeof(Cell));

	// Temporary init of cells
	Cell basicCell;
	basicCell.trace = 0;
	basicCell.foodType = 0;
	basicCell.type = EMPTY_CELL;
	basicCell.typeID = INVALID_ID;

	for(i=0;i<sizeX;i++)
		for(j=0;j<sizeY;j++)
			out->cells[i][j] = basicCell;


	return out;
}

int main(){

	signal(SIGINT, sigHandler);
	Message * sndMsg;
	Message * rcvMsg;


	// MAP LOADER HERE

	World * world;
	world = getWorld(SIZE_X, SIZE_Y, MAX_CONNECTIONS, MAX_TURNS);
	while(1){

		sndMsg = NULL;
		rcvMsg = NULL;

		printf("Waiting to receive...\n\n");
		rcvMsg = receiveMessage(ANT);

		printf("Message received. \n\n");

		printMessage(rcvMsg);

		//parseMessage(rcvMsg, world);

		//if( rcvMsg != NULL){
	//		Pos pos = { 0, 0 };
	//		sndMsg = createMessage(getpid(),  rcvMsg->pidFrom, RECEIVED, OK, pos, 0);
	//		sendMessage(ANT, sndMsg);
	//	}
	}
 }
