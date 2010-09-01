#include "../include/ant.h"

//---------------------------------------------
//FALTA:
//VERIFICAR EL TRACE QUE SE HACE NEGATIVO Y A VECES DECRECE MAS RAPIDO
//DEFINIR LOS LIMITES Y LA INTERACCION ENTRE HORMIGAS JUNTAS
//AGREGAR EL COMPORTAMIENTO PARA LAS BIG INCLUYENDO EL SHOUT
//---------------------------------------------

int 
main(void){
	printf("PID: %d \n", getpid());

	//SET REGISTER -----------------------------
	
	openIPC();
	Ant * ant = malloc(sizeof(Ant));	
	
	getchar();
	setRegister(ant);
	
	ant->food = NO_FOOD;
	ant->opCode = -1;
	Pos pos = {1,0};	

	//COMPORTAMIENTO FORZADO PARA TESTEAR TOMAR Y DEJAR COMIDA
	getchar();
	move(pos, false);
	getchar();
	move(pos, false);
	getchar();
	move(pos, false);
	getchar();
	search(ant);
	getchar();
	if(ant->opCode == FOOD){
		printf("hola\n");
		getNearFood(ant);		
		ant->opCode = -1;
	}


	
	while(getchar() != 'q')
		action(ant);

	closeIPC();
}


int
action(Ant * ant){
	srand (time(NULL)); 
	int i;
	if(ant->opCode == -1){	// If the ant 	
		if(ant->food != NO_FOOD)	// If the ant is carrying food
			goAnthill(ant);
		else {
			if(rand()%10 > 6)
				search(ant);
			else
				randomMove(ant);		
		}
	} else {
		if(ant->opCode == FOOD)
			getNearFood(ant);
		else
			move(ant->mov, false);

		ant->opCode = -1;
	}
	
}


/* In addition of approach to the anthill, the ant leaves trace */
void
goAnthill(Ant * ant){
	Pos anthill = ant->anthill;
	Pos currentPos = getCurrentPos();
	Cardinal card = getCardinal(currentPos, anthill) % 4;	// If NW then go to N, NE go to E, SE go to S and SW go to W
	Pos to;
	printf("CARDINAL: %d\n", card);
	to.x = vecMov[card][0];
	to.y = vecMov[card][1];	
	printf("CURRENT: %d, %d.    ANTHILL: %d, %d.    TO: %d, %d\n", currentPos.x, currentPos.y, anthill.x, anthill.y, to.x, to.y);
	// If the ant is going to arrive to the anthill
	if( anthill.x == currentPos.x+to.x && anthill.y == currentPos.y+to.y ){
		if(setFood(to))
			ant->food = NO_FOOD;
	}
	// Tries to move closer, but if it can't then, move to another direction
	else if( !move(to, true) )
		randomMove(ant);	
}


bool
randomMove(Ant * ant){
	int index, count = 0;
	Pos pos;
	bool tried[4] = {false, false, false, false};	// Verify if the ant tried to go to the i direction
	do{
		do{
			index = rand()%4;
		} while(tried[index]);

		pos.x = vecMov[index][0];
		pos.y = vecMov[index][1];
		
		tried[index] = true;
		count++;

	} while(!move(pos, false) && count < 4);
	
	if(count == 4)
		return false;	// It didn't move
	return true;	// The ant moved
}


bool
move(Pos to, bool trace){
	Message * send;	
	Message * received;

	printf("I want to move to (%d,%d) \n", to.x, to.y);
	if(trace)
		send = createMessage(getpid(), -1, MOVE, SET, to, 1);
	else
		send = createMessage(getpid(), -1, MOVE, SET, to, 0);
	printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER);
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
getCurrentPos(){
	Message * send;
	Message * received;
	Pos pos;
	
	send = createMessage(getpid(), -1, REGISTER, GET, pos, 0);	
	sendMessage(SERVER, send);
	received = receiveMessage(SERVER);
	pos = received->pos;
	return pos;
}


void 
setRegister(Ant * ant){

	Message * send;
	Message * received;

	Pos pos = {0,0};

	// Sending first register
	printf("Trying to register... \n");
	send = createMessage(getpid(), -1, REGISTER, SET, pos, 0);
	printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == REGISTER && received->param == OK ){
		printf("Register successful.\n");
		ant->anthill.x = received->pos.x;
		ant->anthill.y = received->pos.y;
	} else
		printf("Register failed.\n");
}


void
search(Ant * ant){

	int i;
	Pos to;
	Message * send;	
	Message * received;

	// Search food
	for(i = 0; i < 4; i++){
		to.x = vecMov[i][0];
		to.y = vecMov[i][1];
		printf("I want to search food from (%d,%d) \n", to.x, to.y);
		send = createMessage(getpid(), -1, MOVE, GET, to, 0);  
		printMessage(send);
		sendMessage(SERVER, send);

		received = receiveMessage(SERVER);
		printf("Message received.\n");
		printMessage(received);
		if(received->opCode == FOOD && received->param == OCCUPIED){
				ant->opCode = FOOD;
				ant->mov = to;
				return;
		} else if(received->opCode == FOOD && received->param == BIG) // FALTA IMPLEMENTAR
			printf("I found food, but is too big! \n");
		else 
			printf("I didn't find food =( \n");
	}

	// If the ant didn't find food, search trace
	Pos pos;
	double trace = 0;
	for(i = 0; i < 4; i++){
		to.x = vecMov[i][0];
		to.y = vecMov[i][1];
		send = createMessage(getpid(), -1, MOVE, GET, to, 0); // Obtain the trace and verifies if the cell is empty
		printMessage(send);
		sendMessage(SERVER, send);

		received = receiveMessage(SERVER);
		printMessage(received);
		
		if(received->param == EMPTY && received->trace > trace){
				trace = received->trace;
				pos = received->pos;
		}
	}
	if(trace != 0){
		ant->opCode = TRACE;
		ant->mov = pos;
	}	
}


/* Tries to take food from a near cell*/
bool 
getNearFood(Ant * ant){

	int i;
	Pos to = ant->mov;
	Message * send;	
	Message * received;
	
	printf("I want to get food from (%d,%d) \n", to.x, to.y);
	send = createMessage(getpid(), -1, FOOD, GET, to, 0);
	printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER);
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
setFood(Pos to){
	
	Message * send;	
	Message * received;
	
	printf("I want to leave food on anthill in (%d,%d) \n", to.x, to.y);
	send = createMessage(getpid(), -1, FOOD, SET, to, 0);
	printMessage(send);
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER);
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
}
