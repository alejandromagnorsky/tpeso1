#ifndef COMMON
#define COMMON


/*
Description of protocol:

Protocol between map and anthill

Sent by map
	Returned by anthill

FOOD - SET = Ant has delivered small food
	FOOD - OK = Anthill received food.
	FOOD - NOT_OK = Anthill with problems ?  jaja
FOOD - BIG = Ant has delivered BIG food
	FOOD - OK = Anthill received big food.
	FOOD - NOT_OK = Anthill with problems ?  jaja
	
_____________________________________________________

Protocol between ant and map

Sent by map
	No response by ant (inefficient if not)

SHOUT - SET = An ant has shout at pos	

TURN - SET = The ant has a turn to use.

_____________________________________________________

Protocol between map and ant
Format:
Sent by client (ant)
	Returned by server (map)

Note: Every combination not noted here is invalid.

REGISTER ----------------------------------------

REGISTER - SET = Register an ant. Starts on anthill.
	REGISTER - OK = Register successful.
	REGISTER - NOT_OK = Register unsuccessful.
REGISTER - GET = Check if an ant is registered and its pos
	REGISTER - OK = Ant is registered at pos.
	REGISTER - NOT_OK = Ant is not registered.

MOVE --------------------------------------------

MOVE - SET = Try to move to pos, with or without trace
	MOVE - OK = Move accepted, ant moves and leaves trace (0 to 1).
	MOVE - NOT_OK = Cannot move (should check MOVE - GET before)
	TURN - NOT_OK = Ant has to wait
MOVE - GET = Check if moving to pos is possible (also, gets trace!)
	MOVE - EMPTY = Cell at pos is empty, ant can move
	MOVE - NOT_OK = Bad relative position
	MOVE - OCCUPIED = Cell at pos is occupied by ANT_CELL or ANTHILL_CELL
	FOOD - OCCUPIED = Cell at pos is occupied by FOOD_CELL
	TURN - NOT_OK = Ant has to wait

FOOD ---------------------------------------------

FOOD - SET = Try to leave food on ANTHILL_CELL at pos
	FOOD - OK = Food left at anthill.
	FOOD - EMPTY = This ant has no food
	FOOD - NOT_OK = pos is not correct
	TURN - NOT_OK = Ant has to wait

FOOD - GET = Get food from pos
	FOOD - OK = Food got.
	FOOD - BIG = Food very big (need another ant)
	FOOD - NOT_OK = pos not correct, occupied, or empty cell.
	TURN - NOT_OK = Ant has to wait

SHOUT ---------------------------------------------

SHOUT - SET = Shout, broadcasting to all ants
	SHOUT - OK = Shout broadcasted.
	TURN - NOT_OK = Ant has to wait

________________________________________________________________

Protocol between frontend and backend
(first draft)

Sent by backend
	Returned by frontend

REGISTER - SET = Register ant at pos
REGISTER - OK = Finished registering

MOVE - SET = Move ant to fromPos+pos

FOOD - SET = Position small food at pos.
FOOD - BIG = Position big food at pos.

TURN - SET = Turn has passed

SHOUT - SET = Print a shout at pos

*/


typedef enum { REGISTER, MOVE, FOOD , SHOUT, TRACE, TURN } OpCode;

typedef enum { SET, GET, OK, NOT_OK, EMPTY, OCCUPIED, BIG } OpCodeParam;

typedef enum {  SERVER, CLIENT } NodeType;

typedef enum { false, true } bool;

#define MAX_TURNS 10000

#define MAP_ID -1


typedef struct{
	int key;
	void * args;
} PThreadArg;

typedef struct{
	int x;
	int y;
}Pos;

typedef struct{
	int keyTo, keyFrom;
	OpCode opCode;
	OpCodeParam param;
	Pos pos;
	double trace;

	Pos fromPos; // FRONTEND ONLY
} Message;

Message * createMessage(int keyFrom, int keyTo, OpCode opCode, OpCodeParam param, Pos pos, double trace );

void printMessage(Message * msg);

void errorLog(char * s);



#endif
