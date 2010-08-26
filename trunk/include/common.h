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
	Returned by ant

SHOUT - SET = An ant has shout at pos	
	SHOUT - OK = Message received correctly.

TURN - SET = The ant has a turn to use.
	TURN - OK = The ant has understood.

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
REGISTER - GET = Check if an ant is registered.
	REGISTER - OK = Ant is registered at pos.
	REGISTER - NOT_OK = Ant is not registered.

MOVE --------------------------------------------

MOVE - SET = Try to move to pos, leaving specified trace (OR NOT)
	MOVE - OK = Move accepted, ant moves and leaves trace (0 to 1).
	MOVE - NOT_OK = Cannot move (should check MOVE - GET before)
	TURN - NOT_OK = Ant has to wait
MOVE - GET = Check if moving to pos is possible (also, gets trace!)
	MOVE - EMPTY = Cell at pos is empty, ant can move
	MOVE - OCCUPIED = Cell at pos is occupied by ANT_CELL or ANTHILL_CELL
	FOOD - OCCUPIED = Cell at pos is occupied by FOOD_CELL

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

MOVE - SET = Position ant at pos with trace
	MOVE - OK = Accepted.

FOOD - SET = Position small food at pos.
	FOOD - OK = Food positioned.
	FOOD - NOT_OK = Something has gone wrong (should not happen)
FOOD - BIG = Position big food at pos.
	FOOD - OK = Food positioned.
	FOOD - NOT_OK = Something has gone wrong (should not happen)

TURN - SET = Turn has passed
	TURN - OK = Command accepted.

SHOUT - SET = Print a shout at pos
	SHOUT - OK = Shout printed (or played)

*/


typedef enum { REGISTER, MOVE, FOOD , SHOUT, TURN } OpCode;

typedef enum { SET, GET, OK, NOT_OK, EMPTY, OCCUPIED, BIG } OpCodeParam;

typedef enum { ANTHILL, ANT, MAP, FRONTEND } NodeType;

typedef enum { false, true } bool;

#define MAX_CONNECTIONS 50
#define MAX_TURNS 10000

typedef struct{
	int x;
	int y;
}Pos;

typedef struct{
	int pidTo, pidFrom;
	OpCode opCode;
	OpCodeParam param;
	Pos pos;
	double trace;
} Message;

Message * createMessage(int pidFrom, int pidTo, OpCode opCode, OpCodeParam param, Pos pos, double trace );

void printMessage(Message * msg);



#endif
