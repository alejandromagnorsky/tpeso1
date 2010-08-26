typedef enum { EMPTY_CELL, FOOD_CELL, ANT_CELL, ANTHILL_CELL } CellType;
typedef enum { NO_FOOD, SMALL_FOOD, BIG_FOOD } FoodType;

#define INVALID_ID -1

typedef struct Cell{
	double trace;
	FoodType foodType;	// Not mutually exclusive with ants (ants carry food)
	CellType type;
	int typeID;	// the ID of the client currently on the cell, default (empty) -1
} Cell;


// Esto es mejor, usar esto
typedef struct World{
	Cell ** cells;
	int sizeX, sizeY;
	int maxConnections;
	int * clients;
	int turnsLeft;
} World;


World * getWorld( int sizeX, int sizeY, int maxConnections, int turnsLeft);
