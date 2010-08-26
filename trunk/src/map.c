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

#define SIZE_X 15
#define SIZE_Y 15


void printWorld(World * w){
	printf("\n");
	int i,j;
	for(i=0;i<w->sizeX;i++){
		for(j=0;j<w->sizeY;j++)
			switch(w->cells[i][j].type){

				case ANT_CELL:
					printf("A");
					if(w->cells[i][j].foodType == NO_FOOD)
						printf("  |");
					else printf("F |");
					break;
				case ANTHILL_CELL:
					printf("H  |");
					break;
				case FOOD_CELL:
					printf("F  |");
					break;
				default:
					printf("%.1f|", w->cells[i][j].trace);
					break;
			}
		printf("\n");
	}
	printf("\n");
}

void printWorldData(World * w){
	int i;
	printf("Registered ants: ");
	for(i=0;i<w->maxConnections;i++)
		printf("%d ", w->clients[i]);
	printf("\n");

	printf("Ants inside anthill: ");
	for(i=0;i<w->anthill.maxPopulation;i++)
		printf("%d ", w->anthill.ants[i]);
	printf("\n");
}

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
	int i,j;

	ans = createMessage( getpid(), msg->pidFrom, REGISTER, NOT_OK, msg->pos, 0 );

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
			ans = createMessage( getpid(), msg->pidFrom, REGISTER, OK, msg->pos, 0);
			printf("Registered ant %d at (%d,%d) \n", msg->pidFrom, world->anthill.pos.x, world->anthill.pos.y);
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
	ans = createMessage( getpid(), msg->pidFrom, REGISTER, OK, msg->pos, msg->trace);
	if(!exists(msg->pidFrom, world))
			ans = createMessage( getpid(), msg->pidFrom, REGISTER, NOT_OK, msg->pos, msg->trace);

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

	// If Ant exists
	if(exists(msg->pidFrom, world))
		// And the move is valid (neighbor
		if( neighborCells(getAntCellByPID(world, msg->pidFrom)->pos, msg->pos))
	
			// And cell is empty
			if(!isOccupied(&msg->pos, world))
					ans = createMessage( getpid(), msg->pidFrom, MOVE, EMPTY, msg->pos, trace);
			// Or has food
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


bool neighborCells(Pos p1, Pos p2 ){
	if( (abs(p1.x - p2.x) == 1 && p1.y - p2.y == 0 ) 
	   || ( abs(p1.y -p2.y) == 1  && p1.x - p2.x == 0 ))
		return true;
	return false;
}

void setWorldPosition(Message * msg,World * world){

	Message * ans;
	int i;
	ans = createMessage( getpid(), msg->pidFrom, MOVE, NOT_OK, msg->pos, msg->trace);

	// If cell is empty and ant is registered
	if(!isOccupied(&msg->pos, world) && exists(msg->pidFrom, world)){
		// If leaves or not trace 
		if(msg->trace == 1 || msg->trace == 0){
			
			int index =  antExistsInAnthill(world, msg->pidFrom);
			bool neighbor = false;

			Cell * nextCell = &world->cells[msg->pos.x][msg->pos.y];


			// Check if ant is in anthill, then move it
			if( index >= 0){
				// Check if next position is neighbor of anthill
				if(neighborCells(msg->pos, world->anthill.pos)){
					world->anthill.ants[index] = INVALID_ID;
					neighbor = true;
				}
			} else {
				// If ant is in world, erase old cell data, and if food is carried, keep carrying
				Cell * oldCell = getAntCellByPID(world, msg->pidFrom );
				
				// But, check if next pos is neighbor !
				if(neighborCells(msg->pos, oldCell->pos)) {
					oldCell->type = EMPTY_CELL;
					oldCell->typeID = INVALID_ID;
					nextCell->foodType = oldCell->foodType; // carry food if possible
					neighbor = true;
				}
			}

			// If next position is neighbor, then finally send message and manage next cell.
			if(neighbor){
				ans = createMessage( getpid(), msg->pidFrom, MOVE, OK, msg->pos, msg->trace);
				// Set next cell data
				nextCell->type = ANT_CELL;
				nextCell->trace = msg->trace;
				nextCell->typeID = msg->pidFrom;
			}
		}
	}
	sendMessage(ANT, ans);
}


void setFoodAtAnthill(Message * msg, World * world){
	Message * ans;
	ans = createMessage( getpid(), msg->pidFrom, FOOD, NOT_OK, msg->pos, msg->trace);

	// If ant exists
	if(exists(msg->pidFrom, world)){

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

int nextTurn(World * world){

	// Here traces are decreased.
	int i,j;
	for(i=0;i<world->sizeX;i++)
		for(j=0;j<world->sizeY;j++)
			world->cells[i][j].trace -= (world->cells[i][j].trace != 0) ? 0.05 : 0;

	world->turnsLeft--;

	return world->turnsLeft;
}

void getFoodFromWorld(Message * msg, World * world){

	Message * ans;
	ans = createMessage( getpid(), msg->pidFrom, FOOD, NOT_OK, msg->pos, msg->trace);

	// If ant exists
	if(exists(msg->pidFrom, world)){

		Cell * antCell = getAntCellByPID( world,msg->pidFrom);

		// If ant doesn't have food already and requested cell is adjacent
		if(antCell->foodType == NO_FOOD && neighborCells(antCell->pos, msg->pos)){
		
			Cell * foodCell = &world->cells[msg->pos.x][ msg->pos.y];

			// If foodCell has food
			if(foodCell->type == FOOD_CELL ){

				//If it has small food
				if(foodCell->foodType == SMALL_FOOD){
					ans = createMessage( getpid(), msg->pidFrom, FOOD, OK, msg->pos, msg->trace);

					// Take food from cell, give it to ant
					foodCell->type = EMPTY_CELL;
					foodCell->foodType = NO_FOOD; // Doesn't really matter if EMPTY_CELL active
					antCell->foodType = SMALL_FOOD;
					
				} else // If it has big food, warn ant
				if(foodCell->foodType == BIG_FOOD)
					ans = createMessage( getpid(), msg->pidFrom, FOOD, BIG, msg->pos, msg->trace);
			}
		}
	}
	sendMessage(ANT, ans);
}

void broadcastShout(Message * msg, World * world){
	Message * ans;
	int i;

	// If the ant that shout exists
	if(exists(msg->pidFrom, world)){

		// For each ant, send a shout
		for(i=0;i<world->maxConnections;i++)
			if(world->clients[i] != INVALID_ID && world->clients[i] != msg->pidFrom){
				ans = createMessage( getpid(),world->clients[i], SHOUT, SET, msg->pos, msg->trace);
				sendMessage(ANT, ans);
			}

		ans = createMessage( getpid(),msg->pidFrom, SHOUT, OK, msg->pos, msg->trace);
		sendMessage(ANT, ans);
	}
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
		case SHOUT:
			if(msg->param == SET)
				broadcastShout(msg,world);
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

	if(anthillPos.x < sizeX && anthillPos.y < sizeY)
		out->cells[anthillPos.x][anthillPos.y].type = ANTHILL_CELL;
	else
		return NULL; // WTF
	// Here, gotta fork to create the anthill and get its pid
	
	// Testing!
	out->cells[7][5].type = FOOD_CELL;
	out->cells[7][5].foodType = SMALL_FOOD;

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
	
	printf("Anthill position: %d, %d\n", pos.x, pos.y);
	
	// Turn management is more complex, this is a test
	while(nextTurn(world)){

		sndMsg = NULL;
		rcvMsg = NULL;

		printf("Waiting to receive...\n\n");
		rcvMsg = receiveMessage(ANT);

		printf("Message received. \n\n");

		printMessage(rcvMsg);

		parseMessage(rcvMsg, world);

		printWorld(world);
		printWorldData(world);

	}
 }
