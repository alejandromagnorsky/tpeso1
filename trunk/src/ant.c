typedef enum { false, true } bool;
typedef enum { north, east, south, west, northwest, northeast, southeast, southwest } cardinal;
typedef struct{
	int x;
	int y;
}pos_t;

typedef struct{
	bool food;
	pos_t currentPos;
	pos_t anthill;
}ant_t;

int action(ant_t * ant);
void goAnthill(ant_t * ant);
cardinal getCardinal(ant_t * ant);


int
main(){
}

int
action(ant_t * ant){
	int vecPos[4][2] = {{0,1}, {1,0}, {0,-1}, {-1,0}};
	int i;
	if(ant->food) // Si ya tiene comida retorna al hormiguero
		goAnthill(ant);
//	else
//		for(i = 0; i < 4; i++){
		
		
}

/* Ademas de retornar al hormiguero, va dejando rastro */
void
goAnthill(ant_t * ant){
	int vecPos[4][2] = {{0,1}, {1,0}, {0,-1}, {-1,0}};
	//leaveTrace(ant->currentPos);
	cardinal c = getCardinal(ant);
	ant->currentPos.x += vecPos[c%4];
	ant->currentPos.y += vecPos[c%4];
	
}
	


cardinal
getCardinal(ant_t * ant){
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
