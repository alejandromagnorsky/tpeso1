#ifndef MAP_H
#define MAP_H

typedef enum { EMPTY_CELL, FOOD_CELL, ANT_CELL, ANTHILL_CELL } CellType;
typedef enum { NO_FOOD, SMALL_FOOD, BIG_FOOD } FoodType;

#define INVALID_ID -1

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
	int sizeX, sizeY;
	int maxConnections;
	int * clients;
	int turnsLeft;
} World;


World * getWorld( int sizeX, int sizeY, int maxConnections, int turnsLeft, Pos anthillPos);

bool exists(int pid,World * world);

bool isOccupied(Pos * pos, World * world);

void registerAnt(Message * msg, World * world);

void checkRegistered(Message * msg, World * world);

void parseMessage(Message * msg, World * world);

void getWorldPosition(Message * msg,World * world);

#endif
