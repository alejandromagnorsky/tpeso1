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



/*
 * Registering works in this way:
 * It adds both a client and an ant to map and the anthill 
 * with same id, but they are not added to the cell matrix
 * In this way, multiple ants are capable of coexisting 
 * in the same place(anthill). Then, each time an ant	
 * moves away from the anthill, that ant's id is removed from
 * the anthill ant vector.
 * If the ant already exists, it returns NOT_OK
*/
void registerAnt(Message * msg, World * world){

	Message * ans;
	int x,y,i,j;
	x = msg->pos.x;
	y = msg->pos.y;

	ans = createMessage( getpid(), msg->pidFrom, REGISTER, NOT_OK, msg->pos, msg->trace );

	if(!exists(msg->pidFrom, world)){
		// Get next empty ant slot on map
		for(i=0;i<world->maxConnections && world->clients[i] != INVALID_ID;i++);
	
		// Get next empty ant slot on anthill		
		for(j=0;j<world->anthill.maxPopulation && world->anthill.ants[j] != INVALID_ID;j++);
		
		// Register ant on map and anthill
		if( i != world->maxConnections && world->clients[i] == INVALID_ID &&
		    j != world->anthill.maxPopulation && world->anthill.ants[i] == INVALID_ID ){
			world->clients[i] = msg->pidFrom;
			world->anthill.ants[j] = msg->pidFrom;
			ans = createMessage( getpid(), msg->pidFrom, REGISTER, OK, msg->pos, world->cells[x][y].trace);
		}
	}

	sendMessage(ANT, ans);
}


/*
 * Check if client exists
*/
bool exists(int pid,World * world){
	int i;
	for(i=0;i<world->maxConnections;i++)
		if(world->clients[i] == pid)
			return true;
	return false;
}

/*
 * Tell the client if the ant is registered
*/
void checkRegistered(Message * msg, World * world){
	int i;
	Message * ans;
	ans = createMessage( getpid(), msg->pidFrom, REGISTER, NOT_OK, msg->pos, msg->trace);
	if(!exists(msg->pidFrom, world))
			ans = createMessage( getpid(), msg->pidFrom, REGISTER, OK, msg->pos, msg->trace);

	sendMessage(ANT, ans);
}


/*
 * Check if cell at pos is occupied.
*/
bool isOccupied(Pos * pos, World * world){
	return world->cells[pos->x][pos->y].type != EMPTY_CELL;
}


// Get world position, check if its empty or not
void getWorldPosition(Message * msg,World * world){

	Message * ans;
	double trace = world->cells[msg->pos.x][msg->pos.x].trace;
	ans = createMessage( getpid(), msg->pidFrom, MOVE, OCCUPIED, msg->pos, trace);

	if(!isOccupied(&msg->pos, world))
			ans = createMessage( getpid(), msg->pidFrom, MOVE, EMPTY, msg->pos, trace);
	else if( world->cells[msg->pos.x][msg->pos.y].type == FOOD_CELL )
			ans = createMessage( getpid(), msg->pidFrom, FOOD, OCCUPIED, msg->pos, trace);

	sendMessage(ANT, ans);
}

Cell * getAntCellByPID(World * world, int pid ){

	int i,j;

	// If ant is in anthill, return anthill cell
	if(antExistsInAnthill(world, pid) >= 0 )
		return &world->cells[world->anthill.pos.x][world->anthill.pos.y];

	// If ant is in world, return world cell
	else if(exists(pid, world))
		for(i=0;i<world->sizeX;i++)		
			for(j=0;j<world->sizeY;j++)
				if( world->cells[i][j].typeID == pid)
					return &world->cells[i][j];
	return NULL;
}

// Returns index in anthill vector, -1 if not exists
int antExistsInAnthill(World * world, int pid){
	int i;
	for(i=0;i<world->anthill.maxPopulation;i++)
		if(world->anthill.ants[i] == pid)
			return i;
	return -1;
}

void setWorldPosition(Message * msg,World * world){

	Message * ans;
	double trace = msg->trace;
	int i;
	ans = createMessage( getpid(), msg->pidFrom, MOVE, NOT_OK, msg->pos, trace);

	// If cell is empty and ant is registered
	if(!isOccupied(&msg->pos, world) && exists(msg->pidFrom, world)){
			ans = createMessage( getpid(), msg->pidFrom, MOVE, OK, msg->pos, trace);

			int index =  antExistsInAnthill(world, msg->pidFrom);

			Cell * nextCell = &world->cells[msg->pos.x][msg->pos.y];

			// Check if ant is in anthill, then move it
			if( index >= 0){
				world->anthill.ants[index] = INVALID_ID;
	
			} else {
				// If ant is in world, erase old cell data, and if food is carried, keep carrying
				Cell * oldCell = getAntCellByPID(world, msg->pidFrom );
				oldCell->type = EMPTY_CELL;
				oldCell->trace -= 0.1;
				oldCell->typeID = INVALID_ID;
				nextCell->foodType = oldCell->foodType;
			}

			// Set next cell data
			nextCell->type = ANT_CELL;
			nextCell->trace = msg->trace;
			nextCell->typeID = msg->pidFrom;
	}

	sendMessage(ANT, ans);
}

bool neighborCells(Pos p1, Pos p2 ){
	if( abs(p1.x - p2.x) == 1 || abs(p1.y -p2.y) == 1 ) 
		return true;
	return true;
}

void setFoodAtAnthill(Message * msg, World * world){

	int i;
	Message * ans;
	ans = createMessage( getpid(), msg->pidFrom, FOOD, NOT_OK, msg->pos, msg->trace);
	if(!exists(msg->pidFrom, world)){

		// Cell actually occupied by ant
		Cell * antCell = getAntCellByPID(world, msg->pidFrom );

		// If desired pos is anthill and ant is neighbor of anthill...
		if( msg->pos.x == world->anthill.pos.x && msg->pos.y == world->anthill.pos.y
		 && neighborCells(antCell->pos, world->anthill.pos ))
			// Now check if ant has food to deliver
			if( antCell->foodType == NO_FOOD )
				ans = createMessage( getpid(), msg->pidFrom, FOOD, EMPTY, msg->pos, msg->trace);
			else { // DELIVERANCE!
				ans = createMessage( getpid(), msg->pidFrom, FOOD, OK, msg->pos, msg->trace);

				// Now give food to anthill
				Message * sendFood;

				switch( antCell->foodType){
					case BIG_FOOD:
						//sendFood =  createMessage( getpid(), anthillPID, FOOD, SET, msg->pos, msg->trace);
						break;
					case SMALL_FOOD:
						//sendFood =  createMessage( getpid(), anthillPID, FOOD, BIG, msg->pos, msg->trace);
						break;
				}
//				sendMessage(ANTHILL, sendFood);

				// And take food from ant
				antCell->foodType = NO_FOOD;
			}
	}

	sendMessage(ANT, ans);
}

void getFoodFromWorld(Message * msg, World * world){

}

/* Message parser */
void parseMessage(Message * msg, World * world){

	// Parse opCode
	switch(msg->opCode){
		case REGISTER:
			if(msg->param == SET)
				registerAnt(msg, world);
			else if(msg->param == GET)
				checkRegistered(msg, world);
			break;
		case MOVE:
			if(msg->param == GET)
				getWorldPosition(msg, world);
			else if(msg->param == SET)
				setWorldPosition(msg, world);
			break;
		case FOOD:
			if(msg->param == SET)
				setFoodAtAnthill(msg, world);
			if(msg->param == GET)
				getFoodFromWorld(msg, world);
			break;
	}
}

World * getWorld( int sizeX, int sizeY, int maxConnections, int turnsLeft, Pos anthillPos){

	int i,j;
	World * out = malloc(sizeof(World));

	out->sizeX = sizeX;
	out->sizeY = sizeY;
	out->turnsLeft = turnsLeft;
	out->maxConnections = maxConnections;

	out->anthill.pos = anthillPos;
	out->anthill.maxPopulation = maxConnections;


	// Allocation of collections
	out->anthill.ants = malloc(maxConnections * sizeof(int));
	for(i=0;i<maxConnections;i++)
		out->anthill.ants[i] = INVALID_ID;

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
		for(j=0;j<sizeY;j++){
			out->cells[i][j] = basicCell;
			Pos pos = {i,j};
			out->cells[i][j].pos = pos;
		}

	out->cells[anthillPos.x][anthillPos.y].type = ANTHILL_CELL;
	// Here, gotta fork to create the anthill and get its pid
	

	return out;
}

int main(){

	signal(SIGINT, sigHandler);
	Message * sndMsg;
	Message * rcvMsg;


	// MAP LOADER HERE

	World * world;
	Pos pos = { 3, 5 };
	world = getWorld(SIZE_X, SIZE_Y, MAX_CONNECTIONS, MAX_TURNS, pos);	
	while(1){

		sndMsg = NULL;
		rcvMsg = NULL;

		printf("Waiting to receive...\n\n");
		rcvMsg = receiveMessage(ANT);

		printf("Message received. \n\n");

		printMessage(rcvMsg);

		parseMessage(rcvMsg, world);

		//if( rcvMsg != NULL){
	//		Pos pos = { 0, 0 };
	//		sndMsg = createMessage(getpid(),  rcvMsg->pidFrom, RECEIVED, OK, pos, 0);
	//		sendMessage(ANT, sndMsg);
	//	}
	}
 }
