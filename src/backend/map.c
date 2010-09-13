#include "../../include/communication.h"
#include "../../include/map.h"
#include "../../include/anthill.h"
#include "../../include/GameLogic.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

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
			// Gives priority to first ant registered, due to Next Turn algorithm
			world->clients[i].turnLeft = NO_TURN;	
			world->anthill.ants[j] = msg->keyFrom;

			ans = createMessage( MAP_ID, msg->keyFrom, REGISTER, OK, anthillPos, 0);
			//printf("Registered ant %d at (%d,%d) \n", msg->keyFrom, world->anthill.pos.x, world->anthill.pos.y);
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

	// This wastes no turn
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

/*
 * Check if cell at pos is occupied.
*/
bool isOccupiedByAnt(Pos * pos, World * world){
	return world->cells[pos->x][pos->y].type == ANT_CELL;
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

	int clientIndex = getAntIndexByKey(world,msg->keyFrom);

	Pos antPos = getAntCellByPID(world,msg->keyFrom )->pos;
	Pos desiredPos = addPositions(antPos, msg->pos);
	
	
	if( world->clients[clientIndex].turnLeft == NO_TURN ){
		// This should never happen (agreement with ant)
		ans = createMessage( MAP_ID, msg->keyFrom, TURN, NOT_OK, msg->pos, msg->trace);
	// If it has left turn or ghost turn...
	// Check if move is valid
	} else if(withinMapRange(world, desiredPos) && verifyPosition(msg->pos)){

			double trace = world->cells[desiredPos.x][desiredPos.y].trace;

			ans = createMessage( MAP_ID, msg->keyFrom, MOVE, OCCUPIED, msg->pos, msg->trace);

			// And cell is empty
			if(!isOccupied(&desiredPos, world))
					ans = createMessage( MAP_ID, msg->keyFrom, MOVE, EMPTY, msg->pos, trace);
			// Or has food
			else if( world->cells[desiredPos.x][desiredPos.y].type == FOOD_CELL )
					ans = createMessage(MAP_ID, msg->keyFrom, FOOD, OCCUPIED, msg->pos, trace);

			// Wasted GET turn
			world->clients[clientIndex].turnLeft = GHOST_TURN;
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

int addConflictiveMessage(Message * * conflictive, Message * msg, int size){
	int i;
	for(i=0;i<size;i++)
		if(conflictive[i] == NULL ){
			conflictive[i] = msg;
			return 1;
		}
	return 0;
}

int getQtyConflictiveMessages(Message** conflictive, int size){
	int i;
	int qty = 0;
	for(i=0;i<size;i++)
		if(conflictive[i] != NULL)
			qty++;
	return qty;
}

void setWorldPosition(Message * msg,World * world, Message * * conflictive){

	Message * ans;
	ans = createMessage( MAP_ID, msg->keyFrom, MOVE, NOT_OK, msg->pos, msg->trace);

	int clientIndex = getAntIndexByKey(world,msg->keyFrom);

	// If it has turn (no ghost turn)
	if( world->clients[clientIndex].turnLeft != LEFT_TURN ){
		// This should never happen (agreement with ant)
		ans = createMessage( MAP_ID, msg->keyFrom, TURN, NOT_OK, msg->pos, msg->trace);
	} else {
		Cell * antCell = getAntCellByPID(world,msg->keyFrom );

		Pos antPos = antCell->pos;
		Pos desiredPos = addPositions(antPos, msg->pos);

		// If cell is empty and move is valid
		if(withinMapRange(world,desiredPos) && verifyPosition(msg->pos))
		{
			// If leaves or not trace 
			if((int)msg->trace == 1 || (int) msg->trace == 0){

				// If everything is ok except that position is occupied by ANT
				if(isOccupiedByAnt(&desiredPos, world)){
					// Take turn, so map knows it received msg
					world->clients[clientIndex].turnLeft = NO_TURN;
					// there is conflict. Save it for later analysis
					addConflictiveMessage(conflictive, msg, world->maxConnections);
					return;

				// Check if it is occupied by food
				} else if(!isOccupied(&desiredPos, world)){

					int antIndex =  antExistsInAnthill(world, msg->keyFrom);
					Cell * nextCell = &world->cells[desiredPos.x][desiredPos.y];

					Command comm;
					comm.extra.swap = 0; // No swapping here

					// Check if ant is in anthill, then erase it from anthill
					if( antIndex >= 0){
						world->anthill.ants[antIndex] = INVALID_ID;

						// Tell frontend to register ant at new pos,
						// to prevent multi layering.
						comm.fromX = world->anthill.pos.x + msg->pos.x;
						comm.fromY = world->anthill.pos.y + msg->pos.y;
						comm.op = RegisterCommand;	

					} else {
						// If ant is in world, erase old cell data, and if food is carried, keep carrying
						Cell * oldCell = getAntCellByPID(world, msg->keyFrom );
				
						oldCell->type = EMPTY_CELL;
						oldCell->typeID = INVALID_ID;
						nextCell->foodType = oldCell->foodType; // carry food if possible

						// Tell frontend to move ant
						comm.fromX = oldCell->pos.x;
						comm.fromY = oldCell->pos.y;
						comm.toX = msg->pos.x + oldCell->pos.x;
						comm.toY = msg->pos.y + oldCell->pos.y;

						// If it carries food, add another command
						if(nextCell->foodType != NO_FOOD ){
							comm.op = MoveFoodCommand;	
							addCommand(comm);
						}
						comm.op = MoveAntCommand;
					}
	
					printf("P: %d, MP:%d\n", world->points, world->maxPoints);


					addCommand(comm);

					// Set next cell data
					nextCell->type = ANT_CELL;

					// If trace 1 change it, if not, leave it unchanged
					nextCell->trace =( (int) msg->trace == 1) ? msg->trace : nextCell->trace;
					nextCell->typeID = msg->keyFrom;

					world->clients[clientIndex].turnLeft = NO_TURN;
					ans = createMessage( MAP_ID, msg->keyFrom, MOVE, OK, msg->pos, msg->trace);
				}
			}
		}
	}

	sendMessage(CLIENT, ans);
}


void setFoodAtAnthill(Message * msg, World * world){
	Message * ans;
	ans = createMessage( MAP_ID, msg->keyFrom, FOOD, NOT_OK, msg->pos, msg->trace);


	int clientIndex = getAntIndexByKey(world,msg->keyFrom);

	// Cell actually occupied by ant
	Cell * antCell = getAntCellByPID(world, msg->keyFrom );

	Pos desiredPos = addPositions(antCell->pos, msg->pos);

	// If it has no turn...
	if( world->clients[clientIndex].turnLeft != LEFT_TURN ){
		// This should never happen (agreement with ant)
		ans = createMessage( MAP_ID, msg->keyFrom, TURN, NOT_OK, msg->pos, msg->trace);
	// If desired pos is anthill and ant is neighbor of anthill...
	} else if( withinMapRange(world, desiredPos) && verifyPosition(msg->pos) 
		&& desiredPos.x == world->anthill.pos.x && desiredPos.y == world->anthill.pos.y){
		// Now check if ant has food to deliver
		if( antCell->foodType == NO_FOOD )
			ans = createMessage( MAP_ID, msg->keyFrom, FOOD, EMPTY, msg->pos, msg->trace);
		else { // DELIVERANCE!
			ans = createMessage( MAP_ID, msg->keyFrom, FOOD, OK, msg->pos, msg->trace);
	
			// Waste turn
			world->clients[clientIndex].turnLeft = NO_TURN;

			// Tell frontend to destroy food
			Command comm;
			comm.fromX = antCell->pos.x;
			comm.fromY = antCell->pos.y;

			comm.op = DeleteFoodCommand;	
			addCommand(comm);

			// Now give food to anthill
			Message * sendFood;

			switch( antCell->foodType){
				case SMALL_FOOD:
					sendFood =  createMessage( MAP_ID, ANTHILL_KEY, FOOD, SET, msg->pos, msg->trace);
					world->points++;
					break;
				case BIG_FOOD:
					sendFood =  createMessage( MAP_ID, ANTHILL_KEY, FOOD, BIG, msg->pos, msg->trace);
					world->points+=5;
					break;
				default: break;
			}
			sendMessage(CLIENT, sendFood);

			// And take food from ant
			antCell->foodType = NO_FOOD;
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
int getQtyWastedTurns(World * world){
	int i;
	int out = 0;
	for(i=0;i<world->maxConnections;i++)
		if(world->clients[i].key != INVALID_ID &&
			world->clients[i].turnLeft == NO_TURN)
			out++;
	return out;	
}

//Get index of message in which ant wants to exchange with another ant
int getConflictiveIndexByPos(Message * * conflictive, int size, World * world, Pos from, Pos to ){
	int i;
	for(i=0;i<size;i++)
		if(conflictive[i] != NULL ){

			Message * msg = conflictive[i];

			Cell * antCell = getAntCellByPID(world,msg->keyFrom );
			Pos antPos = antCell->pos;

			// Desired position is already validated 
			Pos desiredPos = addPositions(antPos, msg->pos);

			if( desiredPos.x == from.x && desiredPos.y == from.y &&
				antPos.x == to.x && antPos.y == to.y){
				return i;
			}
		}
	return -1;
}

/*
The idea is simple: check for confrontating ants.
If there is a pair of ants who want to exchange places, swap them.
In any other case, just send them NOT_OK messages.

At this point, every ant who could move moved or stayed.
This means that this is a second sweep of message parsing.
Having this in mind is important as context for the code.

Returns quantity of ants that still have turn left. (0 if next turn)

Note 1: This DOES NOT happen when ants are in anthill, so that case is discarded.

Note 2: doing a real conflict analysis could lead to O(n!)
temporal complexity, which
1) is very complex 
2) may not always find an exact solution (may not satisfy every message)
so its easier to just send them negative messages.
*/
int resolveConflicts(Message * * conflictive, int size, World * world){
	
	Message * ans;

	int i;
	for(i=0;i<size;i++)
		if(conflictive[i] != NULL){

			Message * msg = conflictive[i];

			Cell * antCell = getAntCellByPID(world,msg->keyFrom );
			Pos antPos = antCell->pos;

			// Desired position is already validated 
			Pos desiredPos = addPositions(antPos, msg->pos);

			int otherAntIndex = getConflictiveIndexByPos(conflictive, size, world, antPos, desiredPos);

			// If the other ant wishes to exchange,
			if( otherAntIndex != -1 ){

				ans = createMessage( MAP_ID, msg->keyFrom, MOVE, OK, msg->pos, msg->trace);
				sendMessage(CLIENT, ans);

				Message * otherMsg = conflictive[otherAntIndex];

				ans = createMessage( MAP_ID, otherMsg->keyFrom, MOVE, OK, otherMsg->pos, otherMsg->trace);
				sendMessage(CLIENT, ans);

				Cell * otherAnt = getAntCellByPID(world,otherMsg->keyFrom );

				Command comm;

				// First move antCell
				comm.fromX = antCell->pos.x;
				comm.fromY = antCell->pos.y;
				comm.toX = otherAnt->pos.x;
				comm.toY = otherAnt->pos.y;

				// If it carries food, add another command
				if(antCell->foodType != NO_FOOD ){
					comm.op = MoveFoodCommand;	
					comm.extra.swap = 0;
					addCommand(comm);
				}
				comm.extra.swap = 1;
				comm.op = MoveAntCommand;
				addCommand(comm);

				// Now move the other ant
				comm.fromX = otherAnt->pos.x;
				comm.fromY = otherAnt->pos.y;
				comm.toX = antCell->pos.x;
				comm.toY = antCell->pos.y;
		
				// If it carries food, add another command
				if(otherAnt->foodType != NO_FOOD ){
					comm.op = MoveFoodCommand;
					comm.extra.swap = 0;	
					addCommand(comm);
				}
				comm.extra.swap = 1;
				comm.op = MoveAntCommand;
				addCommand(comm);

				printf("Swap ocurred!\n");

				// Now swap them!
				CellType type = otherAnt->type;
				FoodType foodType = otherAnt->foodType;
				int typeID = otherAnt->typeID;

				otherAnt->type = antCell->type;
				otherAnt->foodType = antCell->foodType;
				otherAnt->typeID = antCell->typeID;
				otherAnt->trace =( (int) otherMsg->trace == 1) ? otherMsg->trace : otherAnt->trace;
			
				antCell->type = type;
				antCell->foodType = foodType;
				antCell->typeID = typeID;
				antCell->trace =( (int) msg->trace == 1) ? msg->trace : antCell->trace;

				conflictive[i] = NULL;
				conflictive[otherAntIndex] = NULL;
			// Else, just a negative message response
			} else {
				ans = createMessage( MAP_ID, msg->keyFrom, MOVE, NOT_OK, msg->pos, msg->trace);
				sendMessage(CLIENT, ans);
			}
		}

	int leftAnts = 0;

	// Now give turn to resolved messages
	for(i=0;i<size;i++)
		if(conflictive[i] != NULL){
			Message * leftMsg = conflictive[i];

			int clientIndex = getAntIndexByKey(world,leftMsg->keyFrom);
		
			// Give turn
			world->clients[clientIndex].turnLeft = LEFT_TURN;

			conflictive[i] = NULL;
			leftAnts++;
		}

	return leftAnts;
}

int nextTurn(World * world, Message * * conflictive){

	int active =  getQtyWastedTurns(world);
	Message * turn;

	if( active == world->maxConnections ){
	//	printf("NUEVO TURNO: %d \n", world->turnsLeft);

		//printf("Conflictive: %d\n", getQtyConflictiveMessages(conflictive, world->maxConnections));
		int left = resolveConflicts(conflictive, world->maxConnections, world);
		// If there are ants left, skip turn logic, and wait them 
		if( left ){
		//	printWorldData(world);
			printf("Hay %d hormigas que tienen otro turno!\n", left );
			return 1;
		}

		if(world->points == world->maxPoints)
			return 0;

		Pos tmp = {0,0};

		Command comm;

		// Here traces are decreased.
		int i,j;
		int changedState;
		for(i=0;i<world->sizeX;i++)
			for(j=0;j<world->sizeY;j++){

				// Reset cell state 
				world->cells[i][j].helping = NORMAL_STATE;
				
				changedState = world->cells[i][j].trace > 0.0 ? 1 : 0;
				world->cells[i][j].trace -= (world->cells[i][j].trace > 0.0) ? 0.01 : 0;
				changedState = changedState && (world->cells[i][j].trace <= 0.15);

				if(world->cells[i][j].trace > 0 ){
					comm.op = SetTraceCommand;
					comm.fromX = i;
					comm.fromY = j;
					comm.extra.trace = world->cells[i][j].trace;
					addCommand(comm);
				} else if(changedState){
					comm.op = DeleteTraceCommand;
					comm.fromX = i;
					comm.fromY = j;
					addCommand(comm);
				}
			}


//		printf("NUEVO TURNO: %d \n", world->turnsLeft);
	//	printWorldData(world);

		for(i=0;i<world->maxConnections;i++)
			if(world->clients[i].key != INVALID_ID){
				world->clients[i].turnLeft = LEFT_TURN;
				turn = createMessage( MAP_ID, world->clients[i].key, TURN, SET, tmp, 0);
				sendMessage(CLIENT, turn);
			}
		//sleep(1);

		// Tell frontend turn has ended
		pthread_mutex_lock(&EOT_mutex);

		EOT = 1;

		// Wait till frontend has finished processing and rendering turn
		pthread_cond_wait(&EOT_cond, &EOT_mutex);

		pthread_mutex_unlock(&EOT_mutex);

		world->turnsLeft--;

	}


	return world->turnsLeft;
}

void getFoodFromWorld(Message * msg, World * world){

	Message * ans;
	ans = createMessage( MAP_ID, msg->keyFrom, FOOD, NOT_OK, msg->pos, msg->trace);

	int clientIndex = getAntIndexByKey(world,msg->keyFrom);


	// Cell actually occupied by ant
	Cell * antCell = getAntCellByPID(world, msg->keyFrom );
	Pos desiredPos = addPositions(antCell->pos, msg->pos);

	// If it has no turn...
	if( world->clients[clientIndex].turnLeft != LEFT_TURN ){
		// This should never happen (agreement with ant)
		ans = createMessage( MAP_ID, msg->keyFrom, TURN, NOT_OK, msg->pos, msg->trace);

	// If ant doesn't have food already and requested cell is adjacent
	} else if(withinMapRange(world, desiredPos) && antCell->foodType == NO_FOOD && verifyPosition(msg->pos)){
	
		Cell * foodCell = &world->cells[desiredPos.x][desiredPos.y];

		int antIndex =  antExistsInAnthill(world, msg->keyFrom);

		// If ant IS NOT in anthill, then continue
		if( antIndex == -1 )
			// If foodCell has food
			if(foodCell->type == FOOD_CELL ){

				//If it has small food
				if(foodCell->foodType == SMALL_FOOD){

						ans = createMessage( MAP_ID, msg->keyFrom, FOOD, OK, msg->pos, msg->trace);
						// Take food from cell, give it to ant
						foodCell->type = EMPTY_CELL;
						foodCell->foodType = NO_FOOD; // Doesn't really matter if EMPTY_CELL active
						antCell->foodType = SMALL_FOOD;

						// Tell frontend to move food to ant
						Command comm;
						comm.extra.swap = 0;
						comm.fromX = foodCell->pos.x;
						comm.fromY = foodCell->pos.y;

						comm.toX = antCell->pos.x;
						comm.toY = antCell->pos.y;
						comm.op = MoveFoodCommand;	
						addCommand(comm);

				} else if(foodCell->foodType == BIG_FOOD){

						// If there is no one helping, help
						if(foodCell->helping == NORMAL_STATE){
							foodCell->helping = HELPING_STATE;
							ans = createMessage(MAP_ID, msg->keyFrom, FOOD, BIG, msg->pos, msg->trace);							

						// If there is someone helping, get food
						} else if(foodCell->helping == HELPING_STATE){
							foodCell->helping = NORMAL_STATE;
							
							ans = createMessage( MAP_ID, msg->keyFrom, FOOD, OK, msg->pos, msg->trace);
							// Take food from cell, give it to ant
							foodCell->type = EMPTY_CELL;
							foodCell->foodType = NO_FOOD; // Doesn't really matter if EMPTY_CELL active
							antCell->foodType = BIG_FOOD;

							// Tell frontend to move food to ant
							Command comm;
							comm.extra.swap = 0;
							comm.fromX = foodCell->pos.x;
							comm.fromY = foodCell->pos.y;

							comm.toX = antCell->pos.x;
							comm.toY = antCell->pos.y;
							comm.op = MoveFoodCommand;	
							addCommand(comm);
						}
				}

				// Either take food or wait till someone helps
				world->clients[clientIndex].turnLeft = NO_TURN;
			}
	}

	sendMessage(CLIENT, ans);
}

// WASTE TURN
void setTurn(Message *msg, World * world){
	Message * turn =  createMessage( MAP_ID, msg->keyFrom, TURN, NOT_OK, msg->pos, msg->trace);
	
	int clientIndex = getAntIndexByKey(world, msg->keyFrom);
	if( world->clients[clientIndex].turnLeft > 0 ) 
		world->clients[clientIndex].turnLeft = NO_TURN;

	sendMessage(CLIENT, turn);
}

void setShout(Message *msg, World * world){
	Message * turn =  createMessage( MAP_ID, msg->keyFrom, TURN, NOT_OK, msg->pos, msg->trace);
	
	int clientIndex = getAntIndexByKey(world, msg->keyFrom);
	if( world->clients[clientIndex].turnLeft == LEFT_TURN){

		world->clients[clientIndex].turnLeft = NO_TURN;

		// SHOUT! This is just to waste turn and tell frontend!
		shout();

		turn =  createMessage( MAP_ID, msg->keyFrom, SHOUT, OK, msg->pos, msg->trace);
	}

	sendMessage(CLIENT, turn);
}


/* Message parser */
void parseMessage(Message * msg, World * world, Message * * conflictive){

	// If ant doesn't exist and wants to register
	if(msg->opCode == REGISTER ){
		if(msg->param == SET)
			registerAnt(msg, world);
		else if(msg->param == GET)
			checkRegistered(msg, world);
	// Or if ant exists and wants to do other stuff
	} else if( exists(msg->keyFrom, world) ){
		// Parse opCode
		switch(msg->opCode){
			case MOVE:
				if(msg->param == GET)
					getWorldPosition(msg, world);
				else if(msg->param == SET)
					setWorldPosition(msg, world, conflictive);
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
					setShout(msg, world);
			default: break;
		}
	}
}

void parseError(const char * s){
	fprintf(stderr, "%s : World's file has invalid values.\n", s);
	exit(1);	
}

#define MAX_DIM 50	///////////////////////////////////////// CONSTANTE TEMPORAL; DESPUES BORRAR
/////////////////// bBUENO EN REALIDAD ESTA FUNCION ES TEMPORAL SUPONGO
World * mondoGenerator(){
	Pos	anthillPos;
	int	i, j, x, y, smallFood, bigFood;
	World *	out;

	if ((out = malloc(sizeof(World))) == NULL)
		errorLog("Memory allocation error in world generation.");

	srand((unsigned) time(NULL));

	out->sizeX = (rand() % MAX_DIM) + 10;
	out->sizeY = (rand() % MAX_DIM) + 10;
	anthillPos.x = rand() % out->sizeX;
	anthillPos.y = rand() % out->sizeY;
	out->anthill.pos = anthillPos;
	out->maxConnections =  (rand() % (out->sizeX + out->sizeY  )) + 5;
	out->anthill.maxPopulation = out->maxConnections;;
	out->turnsLeft = MAX_TURNS;
	out->points = 0;
	out->maxPoints = 0;
	smallFood = (rand() % (out->sizeX - 1)) + 1;
	bigFood = (rand() % (out->sizeX - 1)) + 1;

	/* Clients initialization */
	out->anthill.ants = malloc(out->maxConnections * sizeof(int));
	out->clients = malloc(out->maxConnections * sizeof(Client));
	for(i=0;i<out->maxConnections;i++){
		out->anthill.ants[i] = INVALID_ID;
		out->clients[i].key = INVALID_ID;
		out->clients[i].turnLeft = false;
	}

	/* Blank cell initialization */
	Cell blankCell;
	blankCell.trace = 0;
	blankCell.foodType = NO_FOOD;
	blankCell.type = EMPTY_CELL;
	blankCell.typeID = INVALID_ID;
	blankCell.helping = NORMAL_STATE;

	if ( (out->cells = malloc(out->sizeX * sizeof(Cell *))) == NULL)
		errorLog("Memory allocation error in world loading.");

	for(i=0;i<out->sizeX;i++){
		if ( (out->cells[i] = malloc(out->sizeY * sizeof(Cell))) == NULL)
			errorLog("Memory allocation error in world loading.");
		for(j=0;j<out->sizeY;j++){
			out->cells[i][j] = blankCell;
			Pos pos = {i,j};
			out->cells[i][j].pos = pos;
		}
	}

	out->cells[anthillPos.x][anthillPos.y].type = ANTHILL_CELL;

	/* Validate and set small food cells */
	for (i=0; i<smallFood; i++){
		do {
			x = rand() % out->sizeX;
			y = rand() % out->sizeY;
		} while (out->cells[x][y].type == ANTHILL_CELL);
		out->cells[x][y].type = FOOD_CELL;
		out->cells[x][y].foodType = SMALL_FOOD;
		out->maxPoints++;
	}

	/* Validate and set big food cells */
	for (i=0; i<bigFood; i++){
		do {
			x = rand() % out->sizeX;
			y = rand() % out->sizeY;
		} while (out->cells[x][y].type == ANTHILL_CELL);
		out->cells[x][y].type = FOOD_CELL;
		out->cells[x][y].foodType = BIG_FOOD;
		out->maxPoints+=5;
	}

	return out;	
}

World * getWorld(char * filename){
	FILE *	fd;
	Pos	anthillPos;
	int	i, j, x, y, smallFood, bigFood;
	World *	out;

	if ((out = malloc(sizeof(World))) == NULL)
		errorLog("Memory allocation error in world loading.");

	if ( (fd = fopen(filename, "r")) < 0 )
		errorLog("Failed to load world attributes.");

	if ( fscanf(fd, "%d,%d\n%d,%d\n%d\n%d\n", &out->sizeY, &out->sizeX, &anthillPos.y, &anthillPos.x, &out->maxConnections, &smallFood) == EOF)
		errorLog("Failed to read world's file.");
	
	/* Validates world dimensions */
	if (out->sizeX < 0 || out->sizeY < 0)
		parseError("Feel the power of my parser.");

	/* Validates anthill coordinates */
	if (anthillPos.x < 0 || anthillPos.x >= out->sizeX || anthillPos.y < 0 || anthillPos.y >= out->sizeY)
		parseError("Luke, I am your parser.");

	/* Validates small food quantity and max connections value */
	if (smallFood < 0 || out->maxConnections < 0)
		parseError("I've been waiting for you Obi-Wan. We meet again, at last. The circle is now complete.\nWhen I left you, I was but the learner; now I am the parser.");

	out->anthill.pos = anthillPos;
	out->anthill.maxPopulation = out->maxConnections;
	out->turnsLeft = MAX_TURNS;
	out->points = 0;
	out->maxPoints = 0;

	/* Clients initialization */
	out->anthill.ants = malloc(out->maxConnections * sizeof(int));
	out->clients = malloc(out->maxConnections * sizeof(Client));
	for(i=0;i<out->maxConnections;i++){
		out->anthill.ants[i] = INVALID_ID;
		out->clients[i].key = INVALID_ID;
		out->clients[i].turnLeft = false;
	}

	/* Blank cell initialization */
	Cell blankCell;
	blankCell.trace = 0;
	blankCell.foodType = NO_FOOD;
	blankCell.type = EMPTY_CELL;
	blankCell.typeID = INVALID_ID;
	blankCell.helping = NORMAL_STATE;

	if ( (out->cells = malloc(out->sizeX * sizeof(Cell *))) == NULL)
		errorLog("Memory allocation error in world loading.");

	for(i=0;i<out->sizeX;i++){
		if ( (out->cells[i] = malloc(out->sizeY * sizeof(Cell))) == NULL)
			errorLog("Memory allocation error in world loading.");
		for(j=0;j<out->sizeY;j++){
			out->cells[i][j] = blankCell;
			Pos pos = {i,j};
			out->cells[i][j].pos = pos;
		}
	}

	out->cells[anthillPos.x][anthillPos.y].type = ANTHILL_CELL;

	/* Validate and set small food cells */
	for (i=0; i<smallFood; i++){
		if (fscanf(fd, "%d,%d\n", &y, &x) == EOF)
			errorLog("Failed to read world's small food positions.");
		if (out->cells[x][y].type == ANTHILL_CELL || x < 0 || x >= out->sizeX || y < 0 || y >= out->sizeY)
			parseError("I find your lack of faith in my parser disturbing.");
		out->cells[x][y].type = FOOD_CELL;
		out->cells[x][y].foodType = SMALL_FOOD;
		out->maxPoints++;
	}

	/* Read world's big food quantity, then validate it*/
	if (fscanf(fd, "%d", &bigFood) == EOF)
		errorLog("Failed to read world's big food quantity.");
	if (bigFood < 0)
		parseError("Use the Parser, Luke.");

	/* Validate and set big food cells */
	for (i=0; i<bigFood; i++){
		if (fscanf(fd, "\n%d,%d", &y, &x) == EOF)
			errorLog("Failed to read world's big food positions.");
		if (out->cells[x][y].type == ANTHILL_CELL || x < 0 || x >= out->sizeX || y < 0 || y >= out->sizeY)
			parseError("May the parser be with you.");
		out->cells[x][y].type = FOOD_CELL;
		out->cells[x][y].foodType = BIG_FOOD;
		out->maxPoints+= 5;
	}

	if ( fclose(fd) < 0 )
		errorLog("Failed to close world's file descriptor.");

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


void sendDataToFrontend(World * world){
	Command comm;

	int x,y;

	for(x=0;x<world->sizeX;x++)
		for(y=0;y<world->sizeY;y++)
			if(world->cells[x][y].type == FOOD_CELL ){
				if( world->cells[x][y].foodType == SMALL_FOOD){
				comm.fromX = x;
				comm.fromY = y;
				comm.op = RegisterFoodCommand;
				addCommand(comm);
				} else if(world->cells[x][y].foodType == BIG_FOOD ){
				comm.fromX = x;
				comm.fromY = y;
				comm.op = RegisterBigFoodCommand;
				addCommand(comm);

				}
			}

	comm.op = RegisterAnthillCommand;
	comm.fromX = world->anthill.pos.x;
	comm.fromY = world->anthill.pos.y;

	addCommand(comm);
}


void * mapMain(void * arg){

	srand(time(NULL));

	signal(SIGINT, sigHandler);
	Message * rcvMsg;

	// MAP LOADER HERE
	World * world = (World *) arg;

	openServerIPC((void *)world->maxConnections);

	createAnthill(world->maxConnections);

	printf("Ants: %d\n", world->maxConnections);

	sendDataToFrontend(world);

	// Array of pointers
	Message * * conflictive = calloc(world->maxConnections, sizeof(Message *));

	while(nextTurn(world, conflictive)){
		rcvMsg = NULL;
		rcvMsg = receiveMessage(CLIENT,MAP_ID);

		printf("Receiving..\n");
		if(rcvMsg != NULL)
			parseMessage(rcvMsg, world, conflictive);
		printf("Received\n");
		
	}

	closeServer();

	printf("Termino el mapa!\n");

	pthread_exit(NULL);
}
