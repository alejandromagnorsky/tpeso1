#include "../include/ant.h"

//---------------------------------------------
//FALTA:
//VERIFICAR EL COMPORTAMIENTO DE SET FOOD QUE PARECE QUE EL MAPA RESPONDE MAL
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

	
	while(getchar() != 'q')
		action(ant);

	closeIPC();
}


int
action(Ant * ant){
	srand (time(NULL)); 
	int i;
	
	if(ant->food != NO_FOOD)	// If the ant is carrying food
		goAnthill(ant);
	else if(!getNearFood(ant)){	// If the ant couldn't take any food
		if(!followTrace())	// If the ant hasn't any trace around to follow move to a random position
			randomMove(ant);		
	}
	
}


// Tries to move to the maximum trace left by another ant
bool
followTrace(){
	int i, j, k = 0;
	Pos to;
	Message * send;
	Message * receive;
	struct{
		Pos pos;
		double trace;
	} aux[4];
	for(i = 0; i < 4; i++){
		to.x = vecMov[i][0];
		to.y = vecMov[i][1];
		send = createMessage(getpid(), -1, MOVE, GET, to, 0); // Obtain the trace and verifies if the cell is empty
		printMessage(send);
		sendMessage(SERVER, send);

		receive = receiveMessage(SERVER);
		printMessage(receive);
		
		if(receive->param == OK && receive->trace != 0){	//  If the ant can move to a cell which has trace
			// Sort the cells, descending, by their trace
			if(k == 0){
				aux[0].pos = to;
				aux[0].trace = receive->trace;
			} else {
				for(j = 0; j < k; j++)
					if(receive->trace >= aux[j].trace){
						double tmpTrace = aux[j].trace;
						Pos tmpPos = aux[j].pos;
						aux[j].trace = receive->trace;
						aux[j].pos = to;
						aux[j+1].trace = tmpTrace;
						aux[j+1].pos = tmpPos;
						break;
					}
				if(j == k){
					aux[j].trace = receive->trace;
					aux[j].pos = to;
				}
			}
			k++;
		}
	}

	for(i = 0; i < k && !move(aux[i].pos, false); i++)
		;
	
	if(i == k)
		return false;	
	return true;
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
	
	// If the ant is going to arrive to the anthill
	if( anthill.x == currentPos.x+to.x && anthill.y == currentPos.y+to.y )
		setFood(ant);
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


/* Tries to move to any cell wich has food and take it*/
bool 
getNearFood(Ant * ant){

	int i;
	Pos to;
	Message * send;	
	Message * received;
	
	for(i = 0; i < 4; i++){
		to.x = vecMov[i][0];
		to.y = vecMov[i][1];
		printf("I want get food from (%d,%d) \n", to.x, to.y);
		send = createMessage(getpid(), -1, FOOD, GET, to, 0);
		printMessage(send);
		sendMessage(SERVER, send);

		received = receiveMessage(SERVER);
		printf("Message received.\n");
		printMessage(received);
		if(received->opCode == FOOD && received->param == OK && move(to, false)){
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
	sendMessage(SERVER, send);

	received = receiveMessage(SERVER);
	printf("Message received.\n");
	printMessage(received);
	if(received->opCode == FOOD && received->param == OK ){
		printf("I left food at anthill!\n");
		ant->food = NO_FOOD;	
	} else
		printf("I have no food! \n");
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
