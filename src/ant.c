#include <stdlib.h>
#include "../include/common.h"
#include "../include/communication.h"

typedef enum { north, east, south, west, northwest, northeast, southeast, southwest } Cardinal;

typedef struct{
	bool food;
	Pos currentPos;
	Pos anthill;
}Ant;

int vecMov[4][2] = {{0,1}, {1,0}, {0,-1}, {-1,0}}; // Represents: up, right, down and left


int action(Ant * ant);
void goAnthill(Ant * ant);
bool randomMove(Ant * ant);
Cardinal getCardinal(Ant * ant);
bool move(Ant * ant, Pos to);
void leaveTrace(Pos pos);
bool getNearFood(Ant * ant);
bool followTrace(Ant * ant);

int
main(){
}


int
action(Ant * ant){
	srand (time(NULL)); 
	int i;
	Pos * pos;
	pos = malloc(sizeof(Pos));
	if(ant->food) // If the ant is carrying food
		goAnthill(ant);
	else if(!getNearFood(ant)){ // If the ant couldn't take any food
		if(!followTrace(ant)) // If the ant hasn't any trace around to follow move to a random position
			randomMove(ant);		
	}
	
}


// Tries to move to the maximum trace left by another ant
bool
followTrace(Ant * ant){
	int i, j, k;
	Pos pos;
	Message * smsg;
	Message * rmsg;
	struct{
		Pos pos;
		double trace;
	} aux[4];
	for(i = 0; i < 4; i++){
		pos.x = ant->currentPos.x + vecMov[i][0];
		pos.y = ant->currentPos.y + vecMov[i][1];
		//smsg = createMessage(getpid(), -1, TRACE, GET, pos, 0);
		//sendMessage(MAP, smsg);
		//rmsg = receiveMessage(MAP);

		if(rmsg->param == OK && rmsg->trace != 0){ // If the message was received and the cell has trace, sort all
			// Sort the cells, descending, by their trace
			if(k == 0){
				aux[0].pos = pos;
				aux[0].trace = rmsg->trace;
			} else {
				for(j = 0; j < k; j++)
					if(rmsg->trace >= aux[j].trace){
						double tmpTrace = aux[j].trace;
						Pos tmpPos = aux[j].pos;
						aux[j].trace = rmsg->trace;
						aux[j].pos = pos;
						aux[j+1].trace = tmpTrace;
						aux[j+1].pos = tmpPos;
						break;
					}
				if(j == k){
					aux[j].trace = rmsg->trace;
					aux[j].pos = pos;
				}
			}
			k++;
		}
	}

	i = 0;
	while(i < 4 && !move(ant, aux[i].pos))
		i++;
	if(i == 4)
		return false;	
	return true;
}	


bool
getNearFood(Ant * ant){
	int i;
	Pos pos;
	Message * smsg;
	Message * rmsg;
	for(i = 0; i < 4; i++){
		pos.x = ant->currentPos.x + vecMov[i][0];
		pos.y = ant->currentPos.y + vecMov[i][1];
		//smsg = createMessage(getpid(), -1, FOOD, GET, pos, 0);
		//sendMessage(MAP, smsg);
		//rmsg = receiveMessage(MAP);
		if(rmsg->param == OK && move(ant, pos)) // If it found food and can move there
			return true;
	}
	return false;	
}	


/* In addition of approach to the anthill, the ant leaves trace */
void
goAnthill(Ant * ant){
	Pos pos;
	leaveTrace(ant->currentPos);
	Cardinal card = getCardinal(ant) % 4; // If NW then go to N, NE go to E, SE go to S and SW go to W
	
	
	pos.x = ant->currentPos.x + vecMov[card][0];
	pos.y = ant->currentPos.y + vecMov[card][1];
	// First, tries to move closer, but if it can't then, move to another direction
	if( !move(ant, pos) )
		randomMove(ant);		
}


bool
randomMove(Ant * ant){
	int index, count;
	Pos pos;
	bool tried[4] = {false, false, false, false}; // Verify if the ant tried to go to the i direction
	do{
		do{
			index = rand()%4;
		} while(!tried[index]);

		pos.x = ant->currentPos.x + vecMov[index][0];
		pos.y = ant->currentPos.y + vecMov[index][1];
		
		tried[index] = true;
		count++;	
	} while(count < 4 && !move(ant, pos));
	
	if(count == 4)
		return false; // It didn't move
	return true; // The ant moved
}


bool
move(Ant * ant, Pos to){
	//sendMessage();	
}


void
leaveTrace(Pos pos){
	//sendMessage();
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
