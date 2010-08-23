typedef enum { false, true } bool;
typedef enum { north, east, south, west, northwest, northeast, southeast, southwest } Cardinal;
typedef struct{
	int x;
	int y;
}Pos;

typedef struct{
	bool food;
	Pos currentPos;
	Pos anthill;
}Ant;

int action(Ant * ant);
void goAnthill(Ant * ant);
Cardinal getCardinal(Ant * ant);


int
main(){
}

int
action(Ant * ant){
	int vecPos[4][2] = {{0,1}, {1,0}, {0,-1}, {-1,0}};
	int i;
	if(ant->food) // If the ant is carrying food
		goAnthill(ant);
//	else
//		for(i = 0; i < 4; i++){
		
		
}

/* In addition of return to the anthill, the ant leaves trace */
void
goAnthill(Ant * ant){
	int vecPos[4][2] = {{0,1}, {1,0}, {0,-1}, {-1,0}};
	//leaveTrace(ant->currentPos);
	Cardinal c = getCardinal(ant);
	ant->currentPos.x += vecPos[c%4];
	ant->currentPos.y += vecPos[c%4];
	
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
