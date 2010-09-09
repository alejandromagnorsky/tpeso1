#ifndef MAP_H
#define MAP_H

typedef enum { EMPTY_CELL, FOOD_CELL, ANT_CELL, ANTHILL_CELL } CellType;
typedef enum { NO_FOOD, SMALL_FOOD, BIG_FOOD } FoodType;


/*
Valid types of cells:

Small food cell: FOOD_CELL, SMALL_FOOD
Big food cell: FOOD_CELL, BIG_FOOD
Empty cell: EMPTY_CELL, n/a
Ant with no food cell: ANT_CELL, NO_FOOD
Ant with small food cell: ANT_CELL, SMALL_FOOD
Ant with big food cell: ANT_CELL, BIG_FOOD
Anthill: ANTHILL_CELL, n/a

*/

#define INVALID_ID -1

#define NO_TURN 0
#define LEFT_TURN 1
#define GHOST_TURN 2

typedef struct Client{
	int key;
	int turnLeft;
} Client;

typedef struct Cell{
	double trace;
	Pos pos;		// UNMUTABLE In this way, it doesn't matter the structure used by map
	FoodType foodType;	// Not mutually exclusive with ants (ants carry food)
	CellType type;
	int typeID;	// the ID of the client currently on the cell, default (empty) -1
} Cell;

typedef struct Anthill{
	Pos pos;
	int * ants;	// actual ants on anthill
	int maxPopulation;	
} Anthill;

typedef struct World{
	Cell ** cells;
	Anthill anthill;
	int frontendID;
	int sizeX, sizeY;
	int maxConnections;
	Client * clients;
	int turnsLeft;
} World;

void * mapMain(void * arg);

World * getWorld(char * filename);
World * mondoGenerator();

int antExistsInAnthill(World * world, int pid);

bool exists(int pid,World * world);

bool neighborCells(Pos p1, Pos p2 );

bool isOccupied(Pos * pos, World * world);

void registerAnt(Message * msg, World * world);

void checkRegistered(Message * msg, World * world);

void parseMessage(Message * msg, World * world);

Cell * getAntCellByPID(World * world, int pid );

void setWorldPosition(Message * msg,World * world);

void getWorldPosition(Message * msg,World * world);

void setFoodAtAnthill(Message * msg, World * world);

void getFoodFromWorld(Message * msg, World * world);

int withinMapRange(World * world, Pos pos);

int getAntIndexByKey(World * world, int key);

#endif
