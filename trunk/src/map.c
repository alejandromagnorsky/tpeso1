#include "../include/common.h"
#include "../include/communication.h"
#include "../include/map.h"
#include "../include/anthill.h"
#include "../include/GameLogic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE_X 15
#define SIZE_Y 15

void printWorld(World * w){
	printf("\n");
	int i,j;
	for(j=0;j<w->sizeY;j++){
		for(i=0;i<w->sizeX;i++)
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
		printf("%d |", w->clients[i].turnLeft);
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

	Pos anthillPos = world->anthill.pos;

	ans = createMessage( MAP_ID, msg->keyFrom, REGISTER, NOT_OK, anthillPos, 0 );

	if(!exists(msg->keyFrom, world)){
		// Get next empty ant slot on map
		for(i=0;i<world->maxConnections && world->clients[i].key != INVALID_ID;i++);
	
		// Get next empty ant slot on anthill		
		for(j=0;j<world->anthill.maxPopulation && world->anthill.ants[j] != INVALID_ID;j++);
		
		// Register ant on map and anthill
		if( i != world->maxConnections && world->clients[i].key == INVALID_ID &&
		    j != world->anthill.maxPopulation && world->anthill.ants[i] == INVALID_ID ){
			world->clients[i].key = msg->keyFrom;
			world->clients[i].turnLeft = false;
			world->anthill.ants[j] = msg->keyFrom;

			ans = createMessage( MAP_ID, msg->keyFrom, REGISTER, OK, anthillPos, 0);
			printf("Registered ant %d at (%d,%d) \n", msg->keyFrom, world->anthill.pos.x, world->anthill.pos.y);
		}
	}

	sendMessage(CLIENT, ans);
}


/*
 * Check if client exists
*/
bool exists(int key,World * world){
	int i;
	for(i=0;i<world->maxConnections;i++)
		if(world->clients[i].key == key)
			return true;
	return false;
}

/*
 * Tell the client if the ant is registered, return its position
*/
void checkRegistered(Message * msg, World * world){
	Message * ans;

	ans = createMessage( MAP_ID, msg->keyFrom, REGISTER, NOT_OK, msg->pos, msg->trace);
	if(exists(msg->keyFrom, world))
		ans = createMessage( MAP_ID, msg->keyFrom, REGISTER, OK, getAntCellByPID(world,msg->keyFrom )->pos, msg->trace);

	sendMessage(CLIENT, ans);
}

/*
 * Check if cell at pos is occupied.
*/
bool isOccupied(Pos * pos, World * world){
	return world->cells[pos->x][pos->y].type != EMPTY_CELL;
}

bool verifyPosition(Pos pos){
	if((abs(pos.x) == 1 && pos.y == 0) 
        || (abs(pos.y) == 1 && pos.x == 0) )
		return true;
	return false;
}

Pos addPositions(Pos p1, Pos p2){
	Pos out;
	out.x = p1.x + p2.x;
	out.y = p1.y + p2.y;

	return out;
}


// Get world position, check if its empty or not
void getWorldPosition(Message * msg,World * world){

	Message * ans;
	ans = createMessage( MAP_ID, msg->keyFrom, MOVE, NOT_OK, msg->pos, msg->trace);

	if(exists(msg->keyFrom, world)){

		Pos antPos = getAntCellByPID(world,msg->keyFrom )->pos;
		Pos desiredPos = addPositions(antPos, msg->pos);

		// If move is valid
		if(withinMapRange(world, desiredPos) && verifyPosition(msg->pos)){

			double trace = world->cells[desiredPos.x][desiredPos.y].trace;

			ans = createMessage( MAP_ID, msg->keyFrom, MOVE, OCCUPIED, msg->pos, msg->trace);

			// And cell is empty
			if(!isOccupied(&desiredPos, world))
					ans = createMessage( MAP_ID, msg->keyFrom, MOVE, EMPTY, msg->pos, trace);
			// Or has food
			else if( world->cells[desiredPos.x][desiredPos.y].type == FOOD_CELL )
					ans = createMessage(MAP_ID, msg->keyFrom, FOOD, OCCUPIED, msg->pos, trace);
		}
	}
	sendMessage(CLIENT, ans);
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

int withinMapRange(World * world, Pos pos){
	if( pos.x >= 0 && pos.x < world->sizeX && pos.y >= 0 && pos.y < world->sizeY)
		return 1;
	else return 0;
}

int getAntIndexByKey(World * world, int key){
	int i;
	for(i=0;i<world->maxConnections;i++)
		if(world->clients[i].key == key )
			return i;
	return -1;
}

void setWorldPosition(Message * msg,World * world){

	Message * ans;
	ans = createMessage( MAP_ID, msg->keyFrom, MOVE, NOT_OK, msg->pos, msg->trace);

	int clientIndex = getAntIndexByKey(world,msg->keyFrom);

	// First, check if ant exists...
	if( exists(msg->keyFrom, world) && world->clients[clientIndex].turnLeft == true ){
		Cell * antCell = getAntCellByPID(world,msg->keyFrom );

		Pos antPos = antCell->pos;
		Pos desiredPos = addPositions(antPos, msg->pos);

		// If cell is empty and move is valid
		if(withinMapRange(world,desiredPos) &&!isOccupied(&desiredPos, world) 
		&& verifyPosition(msg->pos))
			{
			// If leaves or not trace 
			if((int)msg->trace == 1 || (int) msg->trace == 0){
			
				int antIndex =  antExistsInAnthill(world, msg->keyFrom);
				Cell * nextCell = &world->cells[desiredPos.x][desiredPos.y];

				// Check if ant is in anthill, then erase it from anthill
				if( antIndex >= 0){
					world->anthill.ants[antIndex] = INVALID_ID;

					// Tell frontend to register ant at new pos,
					// to prevent multi layering.
					Command rC;
					rC.fromX = world->anthill.pos.x + msg->pos.x;
					rC.fromY = world->anthill.pos.y + msg->pos.y;
					rC.op = RegisterCommand;	
					rC.valid = 1;

					addRegisterCommand(rC);

				} else {
					// If ant is in world, erase old cell data, and if food is carried, keep carrying
					Cell * oldCell = getAntCellByPID(world, msg->keyFrom );
				
					oldCell->type = EMPTY_CELL;
					oldCell->typeID = INVALID_ID;
					nextCell->foodType = oldCell->foodType; // carry food if possible

					// Tell frontend to move ant
					Command mC;
					mC.fromX = oldCell->pos.x;
					mC.fromY = oldCell->pos.y;
					mC.toX = msg->pos.x + oldCell->pos.x;
					mC.toY = msg->pos.y + oldCell->pos.y;
					mC.op = MoveCommand;	
					mC.valid = 1;

					addMoveCommand(mC);
				}

				// Set next cell data
				nextCell->type = ANT_CELL;

				// If trace 1 change it, if not, leave it unchanged
				nextCell->trace =( (int) msg->trace == 1) ? msg->trace : nextCell->trace;
				nextCell->typeID = msg->keyFrom;
			
				world->clients[clientIndex].turnLeft = false;
				ans = createMessage( MAP_ID, msg->keyFrom, MOVE, OK, msg->pos, msg->trace);
			}
		}
	}

	sendMessage(CLIENT, ans);
}


void setFoodAtAnthill(Message * msg, World * world){
	Message * ans;
	ans = createMessage( MAP_ID, msg->keyFrom, FOOD, NOT_OK, msg->pos, msg->trace);

	// If ant exists
	if(exists(msg->keyFrom, world)){

		// Cell actually occupied by ant
		Cell * antCell = getAntCellByPID(world, msg->keyFrom );

		Pos desiredPos = addPositions(antCell->pos, msg->pos);

		// If desired pos is anthill and ant is neighbor of anthill...
		if( withinMapRange(world, desiredPos) && verifyPosition(msg->pos) 
			&& desiredPos.x == world->anthill.pos.x && desiredPos.y == world->anthill.pos.y){
			// Now check if ant has food to deliver
			if( antCell->foodType == NO_FOOD )
				ans = createMessage( MAP_ID, msg->keyFrom, FOOD, EMPTY, msg->pos, msg->trace);
			else { // DELIVERANCE!
				ans = createMessage( MAP_ID, msg->keyFrom, FOOD, OK, msg->pos, msg->trace);

				// Now give food to anthill
	//			Message * sendFood;

				switch( antCell->foodType){
					case BIG_FOOD:
						//sendFood =  createMessage( getpid(), anthillPID, FOOD, SET, msg->pos, msg->trace);
						break;
					case SMALL_FOOD:
						//sendFood =  createMessage( getpid(), anthillPID, FOOD, BIG, msg->pos, msg->trace);
						break;
					default: break;
				}
//				sendMessage(ANTHILL, sendFood);

				// And take food from ant
				antCell->foodType = NO_FOOD;
			}
		}
	}

	sendMessage(CLIENT, ans);
}
// Qty ants registered
int getQtyRegistered(World * world){
	int i, out;
	out = 0;
	for(i=0;i<world->maxConnections;i++)
		if(world->clients[i].key != INVALID_ID)
			out++;

	return out;
}

// Ants with turn left
int getQtyActiveAnts(World * world){
	int i;
	int out = 0;
	for(i=0;i<world->maxConnections;i++)
		if(world->clients[i].key != INVALID_ID &&
			world->clients[i].turnLeft == true)
			out++;
	return out;	
}

int nextTurn(World * world){


	//printWorldData(world);
	int active =  getQtyActiveAnts(world);
	Message * turn;

	if( active == 0 ){
	//	printf("NUEVO TURNO: %d \n", world->turnsLeft);

		Pos tmp = {0,0};
		// Here traces are decreased.
		int i,j;
		for(i=0;i<world->sizeX;i++)
			for(j=0;j<world->sizeY;j++)
				world->cells[i][j].trace -= (world->cells[i][j].trace > 0) ? 0.01 : 0;

		for(i=0;i<world->maxConnections;i++)
			if(world->clients[i].key != INVALID_ID){
				world->clients[i].turnLeft = true;
				turn = createMessage( MAP_ID, world->clients[i].key, TURN, SET, tmp, 0);
				sendMessage(CLIENT, turn);
			}

		//printWorld(world);
		// Tell frontend turn has ended
		pthread_mutex_lock(&EOT_mutex);

		EOT = 1;

		pthread_cond_wait(&EOT_cond, &EOT_mutex);

		pthread_mutex_unlock(&EOT_mutex);

		world->turnsLeft--;
	}


	return world->turnsLeft;
}

void getFoodFromWorld(Message * msg, World * world){

	Message * ans;
	ans = createMessage( MAP_ID, msg->keyFrom, FOOD, NOT_OK, msg->pos, msg->trace);

	// If ant exists
	if(exists(msg->keyFrom, world)){

		// Cell actually occupied by ant
		Cell * antCell = getAntCellByPID(world, msg->keyFrom );
		Pos desiredPos = addPositions(antCell->pos, msg->pos);


		// If ant doesn't have food already and requested cell is adjacent
		if(withinMapRange(world, desiredPos) && antCell->foodType == NO_FOOD && verifyPosition(msg->pos)){
		
			Cell * foodCell = &world->cells[desiredPos.x][desiredPos.y];

			// If foodCell has food
			if(foodCell->type == FOOD_CELL ){

				//If it has small food
				if(foodCell->foodType == SMALL_FOOD){
					ans = createMessage( MAP_ID, msg->keyFrom, FOOD, OK, msg->pos, msg->trace);

					// Take food from cell, give it to ant
					foodCell->type = EMPTY_CELL;
					foodCell->foodType = NO_FOOD; // Doesn't really matter if EMPTY_CELL active
					antCell->foodType = SMALL_FOOD;
					
				} else // If it has big food, warn ant
				if(foodCell->foodType == BIG_FOOD)
					ans = createMessage(MAP_ID, msg->keyFrom, FOOD, BIG, msg->pos, msg->trace);
			}
		}
	}
	sendMessage(CLIENT, ans);
}

void broadcastShout(Message * msg, World * world){
	Message * ans;
	int i;

	// If the ant that shout exists
	if(exists(msg->keyFrom, world)){

		// For each ant, send a shout
		for(i=0;i<world->maxConnections;i++)
			if(world->clients[i].key != INVALID_ID && world->clients[i].key != msg->keyFrom){
				ans = createMessage( MAP_ID,world->clients[i].key, SHOUT, SET, msg->pos, msg->trace);
				sendMessage(CLIENT, ans);
			}

		ans = createMessage( MAP_ID,msg->keyFrom, SHOUT, OK, msg->pos, msg->trace);
		sendMessage(CLIENT, ans);
	}
}

// WASTE TURN
void setTurn(Message *msg, World * world){
	Message * turn =  createMessage( MAP_ID, msg->keyFrom, TURN, NOT_OK, msg->pos, msg->trace);
	
	int clientIndex = getAntIndexByKey(world, msg->keyFrom);
	if( exists(msg->keyFrom, world) && world->clients[clientIndex].turnLeft == true ) 
		world->clients[clientIndex].turnLeft = false;

	sendMessage(CLIENT, turn);
}


/* Message parser */
void parseMessage(Message * msg, World * world){

	int clientIndex = getAntIndexByKey(world, msg->keyFrom);

	if( (exists(msg->keyFrom, world) && world->clients[clientIndex].turnLeft == true )
	 || !exists(msg->keyFrom, world)  ){
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
			case TURN:
				if(msg->param == SET)
					setTurn(msg, world);
				break;
			case SHOUT:
				if(msg->param == SET)
				//	broadcastShout(msg,world);
					break;
			default: break;
		}
	} else {
		Message * turn =  createMessage( MAP_ID, msg->keyFrom, TURN, NOT_OK, msg->pos, msg->trace);
		sendMessage(CLIENT, turn);

	}

	//world->clients[clientIndex].turnLeft = false;
}
/*
World * getWorld(char * filename){
	FILE *	fd;
	Pos	anthillPos;
	int	i;
	int	smallFood;
	int	bigFood;
	World *	out = malloc(sizeof(World));

	if ( (fd = fopen(filename, "r")) < 0 )
		errorLog("Failed to load world attributes.");

	if ( fscanf(fd, "%d,%d\n%d,%d\n%d\n%d\n", &out->sizeY, &out->sizeX, &anthillPos.y, &anthillPos.x, &out->maxConnections, &smallFood) == EOF)
		errorLog("Failed to read world's file. Corrupted file.");

	out->anthill.pos = anthillPos;

	printf("worldColumns: %d\n", out->sizeY);
	printf("worldRows: %d\n", out->sizeX);
	printf("anthill: x: %d - y:%d\n", out->anthill.pos.x, out->anthill.pos.y);
	printf("ants: %d\n", out->maxConnections);
	printf("smallFood: %d\n", smallFood);

	int temp;
	for (i=0; i<smallFood; i++){
		if (fscanf(fd, "%d,%d\n", &temp, &temp) == EOF)
			errorLog("Failed to read world's small food positions. Corrupted file.");
	}
	if (fscanf(fd, "%d\n", &bigFood) == EOF)
		errorLog("Failed to read world's big food quantity. Corrupted file.");

	for (i=0; i<bigFood; i++){
		if (fscanf(fd, "%d,%d\n", &temp, &temp) == EOF)
			errorLog("Failed to read world's big food positions. Corrupted file.");
	}

	printf("\nFINISH\n");
	fclose(fd);
	return out;
}
*/

World * getWorld( int sizeX, int sizeY, int maxConnections, int turnsLeft){

	int i,j;
	World * out = malloc(sizeof(World));

	out->sizeX = sizeX;
	out->sizeY = sizeY;
	out->turnsLeft = turnsLeft;
	out->maxConnections = maxConnections;

	Pos anthillPos = { 3, 5 };
	out->anthill.pos = anthillPos;
	out->anthill.maxPopulation = maxConnections;


	// Allocation of collections
	out->anthill.ants = malloc(maxConnections * sizeof(int));
	for(i=0;i<maxConnections;i++)
		out->anthill.ants[i] = INVALID_ID;

	out->clients = malloc(maxConnections * sizeof(Client));
	for(i=0;i<maxConnections;i++){
		out->clients[i].key = INVALID_ID;
		out->clients[i].turnLeft = false;
	}

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
	
	// Testing!
	out->cells[7][5].type = FOOD_CELL;
	out->cells[7][5].foodType = SMALL_FOOD;

	return out;
}


void createAnthill(int antCount){
	
        int pid = fork();

        char * antArg = malloc(10*sizeof(char));

        sprintf(antArg, "%d", antCount);

        switch(pid){
                case -1: 
                        printf("can't fork\n");
                        exit(-1);       
                        break;
                case 0 : // this is the code the child runs 
                        execl ("./anthill","anthill","2", antArg, NULL);
                        break;
                default: // this is the code the parent runs 
                        // Get back to map
                        break;
        }

}


void * mapMain(void * arg){

	signal(SIGINT, sigHandler);
	Message * sndMsg;
	Message * rcvMsg;

	createAnthill(5);

	// MAP LOADER HERE
	World * world;
	world = getWorld(SIZE_X, SIZE_Y, 10, MAX_TURNS);	
	//world = getWorld("testmap");	
	
	while(nextTurn(world)){

		sndMsg = NULL;
		rcvMsg = NULL;

//		printf("Waiting to receive...\n\n");
		rcvMsg = receiveMessage(CLIENT,MAP_ID);
//		printf("Received.\n");
	//	printMessage(rcvMsg);
		if(rcvMsg != NULL)
			parseMessage(rcvMsg, world);

	//	printWorldData(world);
	
	}

	pthread_exit(NULL);
}
