#include "../include/ant.h"
#include "../include/common.h"
#include <time.h>
#include <pthread.h>
#include <unistd.h>


int vecMov[4][2] = {{0,1}, {1,0}, {0,-1}, {-1,0}}; // Represents: up, right, down and left




//---------------------------------------------
//FALTA:
//REGISTER GET NO TIENE QUE GASTAR TURNO YA QUE SIRVE PARA OBTENER LA CURRENTPOS

//VERIFICAR EL TRACE QUE SE HACE NEGATIVO Y A VECES DECRECE MAS RAPIDO
//DEFINIR LOS LIMITES Y LA INTERACCION ENTRE HORMIGAS JUNTAS
//AGREGAR EL COMPORTAMIENTO PARA LAS BIG INCLUYENDO EL SHOUT
//---------------------------------------------

void * antMain(void * arg){

	int key = (int)arg;
//	printf("Ant Key: %d \n", key);

	Ant * ant = malloc(sizeof(Ant));	
	ant->food = NO_FOOD;
	ant->opCode = -1;
	ant->key = key;


	setRegister(ant);

//	Pos to = {0,1};
///	Message * send;
//	Message * received;
/*
	while(1){

		received = receiveMessage(SERVER, ant->key);

		if(received->opCode == TURN && received->param == SET){
			printf("Tengo turno: %d\n", key);

			send = createMessage(key, MAP_ID, MOVE, SET, to, 0);
			sendMessage(SERVER, send);

			received = receiveMessage(SERVER, key);

			if(received->opCode == MOVE && received->param != OK){
				send = createMessage(key, MAP_ID, TURN, SET, to, 0);
				sendMessage(SERVER, send);

				received = receiveMessage(SERVER, key);
			}
		}
	}
*/
	printf("Hormiga muriendo!\n");
	pthread_exit(NULL);
}


int
action(Ant * ant){
	if(ant->opCode == -1){	// If the ant 	
		if(ant->food != NO_FOOD)	// If the ant is carrying food
			goAnthill(ant);
		else {
			if(rand()%10 > 6)
				search(ant);
			else
				randomMove(ant, false);		
		}
	} else {
		if(ant->opCode == FOOD)
			getNearFood(ant);
		else
			move(ant->auxPos, false, ant->key);

		ant->opCode = -1;
	}
	return 1;
}


/* In addition of approach to the anthill, the ant leaves trace */
void
goAnthill(Ant * ant){
	Pos anthill = ant->anthill;
	Pos currentPos = getCurrentPos(ant->key);
	Cardinal card = getCardinal(currentPos, anthill) % 4;	// If NW then go to N, NE go to E, SE go to S and SW go to W
	Pos mov, to;
	printf("LLENDO AL HORMIGUERO\n");
	mov.x = vecMov[card][0]; 
	mov.y = vecMov[card][1];
	
	to.x = currentPos.x+mov.x;
	to.y = currentPos.y+mov.y;
	// If the ant is going to arrive to the anthill
	if( anthill.x == to.x && anthill.y == to.y ){
		if(setFood(to, ant->key))
			ant->food = NO_FOOD;
	}
	// Tries to move closer, but if it can't then, move to another direction
	else if( !move(to, true, ant->key) )
		randomMove(ant, true);	
}


bool
randomMove(Ant * ant, bool trace){
	Message * send;
	Message * received;
	Pos currentPos = getCurrentPos(ant->key);
	int index, count = 0;
	Pos to;
	bool tried[4] = {false, false, false, false};	// Verify if the ant tried to go to the i direction
	do{
		do{
			index = rand()%4;
		} while(tried[index]);

		to.x = currentPos.x + vecMov[index][0];
		to.y = currentPos.y + vecMov[index][1];
		
		tried[index] = true;
		count++;

		send = createMessage(ant->key, MAP_ID, MOVE, GET, to, 1);
		sendMessage(SERVER, send);
		received = receiveMessage(SERVER, ant->key);
		if(received->opCode == FOOD && received->param == OCCUPIED){
			ant->auxPos = to;
			getNearFood(ant);
			return true;
		}
			
	} while(!move(to, trace, ant->key) && count < 4);
	
	if(count == 4)
		return false;	// It didn't move
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

	printf("I want to move to (%d,%d) \n", mov.x, mov.y);
	if(trace)
		send = createMessage(key, MAP_ID, MOVE, SET, mov, 1);
	else
		send = createMessage(key, MAP_ID, MOVE, SET, mov, 0);
	printMessage(send);
	sendMessage(SERVER, send);
	printf("Sent message.\n");

	received = receiveMessage(SERVER, key);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == MOVE && received->param == OK ){
		printf("I moved! \n");		
		return true;
	}
	else {
		printf("Couldn't move! =( \n");
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
//	printf("Trying to register... \n");
	send = createMessage(ant->key, MAP_ID, REGISTER, SET, pos, 0);
//	printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER, ant->key);
//	printf("Message received.\n");
//	printMessage(received);
	if(received->opCode == REGISTER && received->param == OK ){
//		printf("Register successful.\n");
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
	Message * received;
	
	// Search food
	for(i = 0; i < 4; i++){
		mov.x = vecMov[i][0];
		mov.y = vecMov[i][1];
		to.x = currentPos.x + mov.x;
		to.y = currentPos.y + mov.y;
		printf("I want to search food from (%d,%d) \n", mov.x, mov.y);
		send = createMessage(ant->key, MAP_ID, MOVE, GET, mov, 0);  
		printMessage(send);
		sendMessage(SERVER, send);

		received = receiveMessage(SERVER, ant->key);
		printf("Message received.\n");
		printMessage(received);
		if(received->opCode == FOOD && received->param == OCCUPIED){
				ant->opCode = FOOD;
				ant->auxPos = to;
				return;
		} else if(received->opCode == FOOD && received->param == BIG) // FALTA IMPLEMENTAR
			printf("I found food, but is too big! \n");
		else 
			printf("I didn't find food =( \n");
	}

	// If the ant didn't find food, search trace
	Pos tmpPos;
	double trace = 0;
	for(i = 0; i < 4; i++){
		mov.x = vecMov[i][0];
		mov.y = vecMov[i][1];
		send = createMessage(ant->key,MAP_ID, MOVE, GET, mov, 0); // Obtain the trace and verifies if the cell is empty
		printMessage(send);
		sendMessage(SERVER, send);

		received = receiveMessage(SERVER, ant->key);
		printMessage(received);
		
		if(received->param == EMPTY && received->trace > trace){
				trace = received->trace;
				tmpPos.x = currentPos.x + mov.x;
				tmpPos.y = currentPos.y + mov.y;
		}
	}
	if(trace != 0){
		ant->opCode = TRACE;
		ant->auxPos = tmpPos;
	}	
}


/* Tries to take food from a near cell*/
bool 
getNearFood(Ant * ant){
	Pos currentPos = getCurrentPos(ant->key);
	Pos to = ant->auxPos;
	Pos mov;
	Message * send;	
	Message * received;
	
	Cardinal card = getCardinal(currentPos, to) % 4;
	mov.x = vecMov[card][0];
	mov.y = vecMov[card][1];

	printf("I want to get food from (%d,%d) \n", mov.x, mov.y);
	send = createMessage(ant->key, MAP_ID, FOOD, GET, mov, 0);
	printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER, ant->key);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == FOOD && received->param == OK){
			printf("I get food! \n");
			ant->food = SMALL_FOOD;
			return true;
	} else if(received->opCode == FOOD && received->param == BIG)
			printf("I cannot get food, too big! \n");
	else 
			printf("I cannot get food =( \n");
	return false;
}


bool 
setFood(Pos to, int key){
	Pos currentPos = getCurrentPos(key);
	Pos mov;
	Message * send;	
	Message * received;

	Cardinal card = getCardinal(currentPos, to) % 4;
	mov.x = vecMov[card][0];
	mov.y = vecMov[card][1];
	
	printf("I want to leave food on anthill in (%d,%d) \n", mov.x, mov.y);
	send = createMessage(key, MAP_ID, FOOD, SET, mov, 0);
	printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER,key);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == FOOD && received->param == OK ){
		printf("I left food at anthill!\n");
		return true;
	} else {
		printf("I have no food! \n");
		return false;
	}
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