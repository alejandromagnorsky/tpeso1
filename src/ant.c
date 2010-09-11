#include "../include/ant.h"
#include "../include/common.h"

int vecMov[4][2] = {{0,1}, {1,0}, {0,-1}, {-1,0}}; // Represents: up, right, down and left
pthread_mutex_t shoutMutex;
pthread_cond_t shoutCond;
int antsToWait = -1;
int futureScreams = 0;
int currentScreams = 0;
int antsQuantity;
int _count = 0;

//---------------------------------------------
//FALTA:
//DEFINIR LA INTERACCION ENTRE HORMIGAS JUNTAS
//AGREGAR EL COMPORTAMIENTO PARA SHOUT
//---------------------------------------------

void * antMain(void * arg){

	int key = (int)arg;
	//printf("Ant Key: %d \n", key);

	Ant * ant = malloc(sizeof(Ant));        
    ant->food = false;
	ant->op = -1;
	ant->key = key;

	setRegister(ant);

	Pos to = {0,0};
	Message * send;
	Message * received;

	while(1){

		received = receiveMessage(SERVER, ant->key);
		
		if(received->opCode == TURN && received->param == SET){
		//	printf("Tengo turno: %d\n", key);

			if(!action(ant)){ // If the ant didn't take any action that consumes a turn, then waste it
				send = createMessage(key, MAP_ID, TURN, SET, to, 0);
				sendMessage(SERVER, send);

				received = receiveMessage(SERVER, key);
			}
		}
	}

	printf("Hormiga muriendo!\n");
	pthread_exit(NULL);
}


bool
action(Ant * ant){
	pthread_mutex_lock(&shoutMutex);

	/*if(antsToWait == -1){ // Initialize
		antsToWait = futureScreams+currentScreams;	// Quantity of ants that has to update the intensity of their screams
		printf("ANTSTOWAIT: %d\n", antsToWait);
	}
	printf("Future: %d. Current: %d\n", futureScreams, currentScreams);
	reduceScreamIntensity(ant);		// Validate or delete the scream if the ant has shouted
	printf("KEY: %d. antsToWait = %d\n", ant->key, antsToWait);
	if(antsToWait > 0)
		pthread_cond_wait(&shoutCond, &shoutMutex);
	else 
		pthread_cond_broadcast(&shoutCond);
	*/
	
	printf("Ant Key: %d \n", ant->key);
	_count++;
	if(_count == antsQuantity){ // The last ant in the turn
		printf("TERMINO TURNO\n");
		_count = 0;
		antsToWait = -1;
	}
	pthread_mutex_unlock(&shoutMutex);

/*
	if(ant->op != -1 || ant->food){	// If the ant isn't doing anything
		if(getNearestScream(ant))	// If the ant has any partner who shouted near, go to help
			ant->op = FOLLOW_SHOUT;
	}*/
	
		

	


	if(ant->op == -1){	// If the ant 	
		if(ant->food)	// If the ant is carrying food
			return goAnthill(ant);
		else {
			if(rand()%100 < 40){
				search(ant);
				return false;
			} else
				return randomMove(ant, false);
		}
	} else {
		switch(ant->op){
		case GET_FOOD:
			ant->op = -1;
			return getNearFood(ant, ant->auxPos);	// Get the food that the ant found in the previous turn
		case FOLLOW_TRACE:
			if(rand()%100 < 80)
				ant->op = SMELL; // The next turn use search
			else
				ant->op = -1;
			return move(ant->auxPos, false, ant->key);	// Follows the trace
		case SMELL:	// The ant has to search
			ant->op = -1;
			search(ant);
			return false;
		case SET_SHOUT:
			setShout(ant);
			ant->op = GET_FOOD;
			return true;
		default:
			return followShout(ant);
		}
	}
}


/* In addition of approach to the anthill, the ant leaves trace */
bool
goAnthill(Ant * ant){
	Pos anthill = ant->anthill;
	Pos currentPos = getCurrentPos(ant->key);
	Cardinal card = getCardinal(currentPos, anthill) % 4;	// If NW then go to N, NE go to E, SE go to S and SW go to W
	Pos mov, to;
	mov.x = vecMov[card][0]; 
	mov.y = vecMov[card][1];
	
	to.x = currentPos.x+mov.x;
	to.y = currentPos.y+mov.y;
	// If the ant is going to arrive to the anthill
	if( anthill.x == to.x && anthill.y == to.y ){
		return setFood(ant, to);
	}
	// Tries to move closer, but if it can't then, move to another direction
	else if( !move(to, true, ant->key) ){
		return randomMove(ant, true);
	}
	return true;
}


bool
randomMove(Ant * ant, bool trace){
	Pos currentPos = getCurrentPos(ant->key);
	int index, count = 0;
	Pos to;
	bool moved;
	bool tried[4] = {false, false, false, false};	// Verify if the ant tried to go to the i direction
	do{
		do{
			index = rand()%4;
		} while(tried[index]);

		to.x = currentPos.x + vecMov[index][0];
		to.y = currentPos.y + vecMov[index][1];
		
		tried[index] = true;
		count++;
		if(!ant->food && ant->op != FOLLOW_SHOUT){
			if(getNearFood(ant, to))
				return true; // The ant took food
		}
	} while(!(moved=move(to, trace, ant->key)) && count < 4);
	
	if(!moved && count == 4)
		return false;	// It didn't move or take food
	return true;	// The ant moved
}


bool
move(Pos to, bool trace, int key){
	Message * send;	
	Message * received;
	Pos mov;
	Pos currentPos = getCurrentPos(key);

	Cardinal card = getCardinal(currentPos, to) % 4;	// If NW then go to N, NE go to E, SE go to S and SW go to W
	mov.x = vecMov[card][0];
	mov.y = vecMov[card][1];	

	//printf("I want to move to (%d,%d) \n", mov.x, mov.y);
	if(trace)
		send = createMessage(key, MAP_ID, MOVE, SET, mov, 1);
	else
		send = createMessage(key, MAP_ID, MOVE, SET, mov, 0);
	//printMessage(send);
	sendMessage(SERVER, send);
	//printf("Sent message.\n");

	received = receiveMessage(SERVER, key);
	//printMessage(received);
	if(received->opCode == MOVE && received->param == OK ){
	//	printf("I moved! \n");	
		return true;
	} else {
	//	printf("Couldn't move! =( \n");
		return false;
	}
}


Pos
getCurrentPos(int key){
	Message * send;
	Message * received;
	Pos pos = {0,0};
	
	send = createMessage(key, MAP_ID, REGISTER, GET, pos, 0);	
	sendMessage(SERVER, send);
	received = receiveMessage(SERVER, key);
	pos = received->pos;
	return pos;
}


void 
setRegister(Ant * ant){
	Message * send;
	Message * received;

	Pos pos = {0,0};

	// Sending first register
	//printf("Trying to register... \n");
	send = createMessage(ant->key, MAP_ID, REGISTER, SET, pos, 0);
	//printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER, ant->key);
	//printMessage(received);
	if(received->opCode == REGISTER && received->param == OK ){
		//printf("Register successful.\n");
		ant->anthill.x = received->pos.x;
		ant->anthill.y = received->pos.y;
	} else
		printf("Register failed.\n");
}


void
search(Ant * ant){
	int i;
	Pos currentPos = getCurrentPos(ant->key);
	Pos mov, to;
	Message * send;	
	Message * received[4];
	
	// Search food
	for(i = 0; i < 4; i++){
		mov.x = vecMov[i][0];
		mov.y = vecMov[i][1];
		to.x = currentPos.x + mov.x;
		to.y = currentPos.y + mov.y;
		//printf("I want to search food from (%d,%d) \n", mov.x, mov.y);
		send = createMessage(ant->key, MAP_ID, MOVE, GET, mov, 0);  
		//printMessage(send);
		sendMessage(SERVER, send);

		received[i] = receiveMessage(SERVER, ant->key);
		//printMessage(received[i]);
		if(received[i]->opCode == FOOD && received[i]->param == OCCUPIED){
				ant->op = GET_FOOD;
				ant->auxPos = to;
				return;
		}
	}

	// If the ant didn't find food, search trace
	Pos tmpPos;
	double trace = 2;
	for(i = 0; i < 4; i++){
		mov.x = vecMov[i][0];
		mov.y = vecMov[i][1];
			
		if(received[i]->param == EMPTY && received[i]->trace > 0 && received[i]->trace < trace){
				trace = received[i]->trace;
				tmpPos.x = currentPos.x + mov.x;
				tmpPos.y = currentPos.y + mov.y;
		}
	}
	if(trace < 1){ // If trace was changed
		ant->op = FOLLOW_TRACE;
		ant->auxPos = tmpPos;
	}	
}


/* Tries to take food */
bool 
getNearFood(Ant * ant, Pos to){
	Pos currentPos = getCurrentPos(ant->key);
	if(currentPos.x == ant->anthill.x && currentPos.y == ant->anthill.y) // Ants cannot get food from the anthill
		return false;
	
	Pos mov;
	Message * send;	
	Message * received;
	
	Cardinal card = getCardinal(currentPos, to) % 4;
	mov.x = vecMov[card][0];
	mov.y = vecMov[card][1];

	//printf("I want to get food from (%d,%d) \n", mov.x, mov.y);
	send = createMessage(ant->key, MAP_ID, FOOD, GET, mov, 0);
	//printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER, ant->key);
	//printMessage(received);
	if(received->opCode == FOOD && received->param == OK){
		//printf("I get food! \n");
		ant->food = true;
		return true;
	} else if(received->opCode == FOOD && received->param == BIG){
		//printf("I cannot get food, too big! \n");
		/*if(!hasShouted(ant, to)) // If the ant didn't shout yet about that big food, shout in the next turn
			ant->op = SET_SHOUT;
		else*/
			ant->op = GET_FOOD;
		ant->auxPos = to;
		return true;
	}
	//else 
	//		printf("I cannot get food =( \n");
	return false;
}


bool 
setFood(Ant * ant, Pos to){
	Pos currentPos = getCurrentPos(ant->key);
	Pos mov;
	Message * send;	
	Message * received;

	Cardinal card = getCardinal(currentPos, to) % 4;
	mov.x = vecMov[card][0];
	mov.y = vecMov[card][1];
	
	//printf("I want to leave food on anthill in (%d,%d) \n", mov.x, mov.y);
	send = createMessage(ant->key, MAP_ID, FOOD, SET, mov, 0);
	//printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER, ant->key);
	//printMessage(received);
	if(received->opCode == FOOD && received->param == OK ){
		printf("I left food at anthill!\n");
		ant->food = false;
		return true;
	} else {
		printf("I have no food! \n");
		return false;
	}
}


void
setShout(Ant * ant){
	int index = ant->key - 3;
	
	Message * send;	
	Message * received;
	Pos mov = {0,0};

	send = createMessage(ant->key, MAP_ID, SHOUT, SET, mov, 0);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER, ant->key);
	if(received->opCode == SHOUT && received->param == OK){
		screams[index].intensity = 1;
		screams[index].pos = ant->auxPos;
		futureScreams++;
		printf("KEY: %d. Intensity: %d. Pos: (%d,%d) \n", ant->key, screams[index].intensity, screams[index].pos.x, screams[index].pos.y);
	} else
		printf("I cannot shout!\n");
	
}


bool
followShout(Ant * ant){
	Pos currentPos = getCurrentPos(ant->key);
	Cardinal card = getCardinal(currentPos, ant->auxPos) % 4;
	Pos mov, to;
	mov.x = vecMov[card][0]; 
	mov.y = vecMov[card][1];
	
	to.x = currentPos.x+mov.x;
	to.y = currentPos.y+mov.y;
	// If the ant is going to arrive to the big food
	if( ant->auxPos.x == to.x && ant->auxPos.y == to.y ){
		ant->op = -1;
		return getNearFood(ant, to);
	}
	// Tries to move closer, but if it can't then, move to another direction
	else if( !move(to, false, ant->key) ){
		return randomMove(ant, false);
	}
	return true;
}


bool
getNearestScream(Ant * ant){
	int i = 0, j = 0;
	Pos currentPos = getCurrentPos(ant->key);
	Pos tmpPos;
	int tmpDist = 15; // Starts with the maximun distance
	while(j < currentScreams){
		if(screams[i].intensity == 0){
			j++;
			if(getDistance(currentPos, screams[i].pos) < tmpDist){
				tmpPos = screams[i].pos;
				tmpDist = getDistance(currentPos, tmpPos);
			}
		}
		i++;
	}
	if(tmpDist != 15){
		ant->op = FOLLOW_SHOUT;
		ant->auxPos = tmpPos;
		return true;
	}
	return false;
}


bool
hasShouted(Ant * ant, Pos to){
	int index = ant->key - 3;
	return ant->op == FOLLOW_SHOUT || (screams[index].pos.x == to.x && screams[index].pos.y == to.y);
}


void
reduceScreamIntensity(Ant * ant){
	int index = ant->key - 3;
	if(screams[index].intensity > -1){
		if(screams[index].intensity == 1){
			futureScreams--;
			currentScreams++;
		} else if(screams[index].intensity == 0){
			currentScreams--;
			screams[index].intensity--;
		}		
		screams[index].intensity--;
		antsToWait--;
	}
}


int
getDistance(Pos from, Pos to){
	return abs(to.x-from.x) + abs(to.y-from.y);
}


Cardinal
getCardinal(Pos from, Pos to){
	int disX = to.x - from.x;
	int disY = to.y - from.y;
	if(disX == 0){
		if(disY > 0)
			return north;
		else
			return south;
	} else if(disY == 0){
		if(disX > 0)
			return east;
		else
			return west;
	} else if(disX > 0){
		if(disY > 0)
			return northeast;
		else
			return southeast;	
	} else if(disX < 0){
		if(disY > 0)
			return northwest;
		else
			return southwest;
	}
	return north;
}
