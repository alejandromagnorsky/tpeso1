#include "../include/ant.h"

int 
main(void){
	printf("PID: %d \n", getpid());

	//SET REGISTER -----------------------------
	
	getchar();
	setRegister();

	Ant * ant = malloc(sizeof(Ant));
	
	ant->food = NO_FOOD;
	//Asumiendo la posicion del anthill
	ant->currentPos; 
	// En la matriz, (3,5)
	ant->currentPos.x = 5;
	ant->currentPos.y = 11;
	ant->anthill = ant->currentPos;
	
	action(ant);
}


int
action(Ant * ant){
	srand (time(NULL)); 
	int i;
	
	if(ant->food != NO_FOOD)	// If the ant is carrying food
		goAnthill(ant);
	else if(!getNearFood(ant)){	// If the ant couldn't take any food
		if(!followTrace(ant))	// If the ant hasn't any trace around to follow move to a random position
			randomMove(ant);		
	}
	
}


// Tries to move to the maximum trace left by another ant
bool
followTrace(Ant * ant){
/*
	int i, j, k;
	Pos pos;
	Message * send;
	Message * receive;
	struct{
		Pos pos;
		double trace;
	} aux[4];
	for(i = 0; i < 4; i++){
		pos.x = ant->currentPos.x + vecMov[i][0];
		pos.y = ant->currentPos.y + vecMov[i][1];
		send = createMessage(getpid(), -1, TRACE, GET, pos, 0);
		sendMessage(MAP, send);
		receive = receiveMessage(MAP);
		printMessage(receive);

		if(receive->param == OK && receive->trace != 0){	//  If the ant can move to a cell which has trace
			// Sort the cells, descending, by their trace
			if(k == 0){
				aux[0].pos = pos;
				aux[0].trace = receive->trace;
			} else {
				for(j = 0; j < k; j++)
					if(receive->trace >= aux[j].trace){
						double tmpTrace = aux[j].trace;
						Pos tmpPos = aux[j].pos;
						aux[j].trace = receive->trace;
						aux[j].pos = pos;
						aux[j+1].trace = tmpTrace;
						aux[j+1].pos = tmpPos;
						break;
					}
				if(j == k){
					aux[j].trace = receive->trace;
					aux[j].pos = pos;
				}
			}
			k++;
		}
	}

	i = 0;
	while(i < k && !move(ant, aux[i].pos))
		i++;
	if(i == k)
		return false;	
	return true;
*/
	//---------------------------------------------	
	//PEDIR QUE EL MAP MANEJE TRACE PARA FACILITAR goAnthill()
	//---------------------------------------------	

	return false;
}	


/* In addition of approach to the anthill, the ant leaves trace */
void
goAnthill(Ant * ant){
	Pos pos;
	leaveTrace(ant->currentPos);
	Cardinal card = getCardinal(ant) % 4;	// If NW then go to N, NE go to E, SE go to S and SW go to W
		
	pos.x = ant->currentPos.x + vecMov[card][0];
	pos.y = ant->currentPos.y + vecMov[card][1];
	// If the ant is going to arrive to the anthill
	if( ant->anthill.x == pos.x && ant->anthill.y == pos.y )
		setFood(ant);
	// Tries to move closer, but if it can't then, move to another direction
	else if( !move(ant, pos) )
		randomMove(ant);	
}


bool
randomMove(Ant * ant){
	int index, count = 0;
	Pos pos;
	Pos aux;
	aux = pos;
	bool tried[4] = {false, false, false, false};	// Verify if the ant tried to go to the i direction
	do{
		do{
			index = rand()%4;
		} while(tried[index]);
		
		pos.x = aux.x;
		pos.y = aux.y;
		pos.x = ant->currentPos.x + vecMov[index][0];
		pos.y = ant->currentPos.y + vecMov[index][1];
		
		tried[index] = true;
		count++;

	} while(!move(ant, pos) && count < 4);
	
	if(count == 4)
		return false;	// It didn't move
	return true;	// The ant moved
}


bool
move(Ant * ant, Pos to){
	Message * send;	
	Message * received;

	printf("I want to move to (%d,%d) \n", to.x, to.y);
	send = createMessage(getpid(), -1, MOVE, SET, to, 0);
	printMessage(send);
	sendMessage(MAP, send);

	received = receiveMessage(MAP);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == MOVE && received->param == OK ){
		printf("I moved! \n");
		ant->currentPos = received->pos;
		return true;
	}
	else {
		printf("Couldn't move! =( \n");
		return false;
	}
}


void
leaveTrace(Pos pos){
	Message * send;
	Message * receive;

	send = createMessage(getpid(), -1, TRACE, SET, pos, 1);
	sendMessage(MAP, send);
	receive = receiveMessage(MAP);
	printMessage(receive);
	//---------------------------------------------	
	//PEDIR QUE EL MAP MANEJE TRACE PARA FACILITAR goAnthill()
	//---------------------------------------------	
}


void 
setRegister(){

	Message * send;
	Message * received;

	Pos pos = {0,0};

	// Sending first register
	printf("Trying to register... \n");
	send = createMessage(getpid(), -1, REGISTER, SET, pos, 0);
	printMessage(send);
	sendMessage(MAP, send);

	received = receiveMessage(MAP);
	printf("Message received.\n");
	//---------------------------------------------	
	//DEBERIA RECIBIR EN EL MENSAJE LA POS DEL ANTHILL
	//---------------------------------------------	
	printMessage(received);
	if(received->opCode == REGISTER && received->param == OK )
		printf("Register successful.\n");
	else
		printf("Register failed.\n");
}


/* Tries to move to any cell wich has food and take it*/
bool 
getNearFood(Ant * ant){

	//---------------------------------------------	
	//LA HORMIGA SOLO PUEDE MOVERSE A UN LUGAR CON COMIDA SI NO TIENE
	//ES DECIR, QUE SI LLEGA A UNA CELDA QUE TIENE COMIDA Y ELLA NO TIENE,
	//LA VA A TOMAR. TAMBIEN FALTA AGREGAR EL COMPORTAMIENTO PARA LAS BIG
	//---------------------------------------------
	int i;
	Pos pos;
	Message * send;	
	Message * received;
	
	for(i = 0; i < 4; i++){
		pos.x = ant->currentPos.x + vecMov[i][0];
		pos.y = ant->currentPos.y + vecMov[i][1];
		printf("I want get food from (%d,%d) \n", pos.x, pos.y);
		send = createMessage(getpid(), -1, FOOD, GET, pos, 0);
		printMessage(send);
		sendMessage(MAP, send);

		received = receiveMessage(MAP);
		printf("Message received.\n");
		printMessage(received);
		if(received->opCode == FOOD && received->param == OK && move(ant, pos)){
				printf("I got food! \n");
				ant->food = SMALL_FOOD;
				return true;
		} else if(received->opCode == FOOD && received->param == BIG)
			printf("I cannot get food, too big! \n");
		else 
			printf("I cannot get food =( \n");
	}
	return false;
}


void 
setFood(Ant * ant){
	
	Message * send;	
	Message * received;

	printf("I want to leave food on anthill in (%d,%d) \n", ant->anthill.x, ant->anthill.y);
	send = createMessage(getpid(), -1, FOOD, SET, ant->anthill, 0);
	printMessage(send);
	sendMessage(MAP, send);

	received = receiveMessage(MAP);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == FOOD && received->param == OK ){
		printf("I left food at anthill!\n");
		ant->food = NO_FOOD;	
	} else
		printf("I have no food! \n");
}


Cardinal
getCardinal(Ant * ant){
	int disX = ant->anthill.x - ant->currentPos.x;
	int disY = ant->anthill.y - ant->currentPos.y;
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
