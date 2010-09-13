#ifndef COMMON
#define COMMON


/*
Description of protocol:

Protocol between map and anthill

Sent by map
	No response by anthill (no need in this app)

FOOD - SET = Ant has delivered small food
FOOD - BIG = Ant has delivered BIG food

_____________________________________________________

Protocol between ant and map

Sent by map
	No response by ant (inefficient if not)

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

TURN ---------------------------------------------------

TURN - GET = Try to get turn
	TURN - OK = Ant has turn
	TURN - NOT_OK = Ant has to wait for turn

SHOUT ---------------------------------------------

SHOUT - SET = Shout, broadcasting to all ants
	SHOUT - OK = Shout broadcasted.
	TURN - NOT_OK = Ant has to wait

*/

typedef enum { false, true } bool;

typedef enum { REGISTER, MOVE, FOOD , SHOUT, TRACE, TURN, EXIT } OpCode;

typedef enum { SET, GET, OK, NOT_OK, EMPTY, OCCUPIED, BIG } OpCodeParam;

typedef enum {  SERVER, CLIENT } NodeType;

#define MAX_TURNS 10000

#define MAP_ID -1
#define ANTHILL_KEY 2

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

} Message;

Message * createMessage(int keyFrom, int keyTo, OpCode opCode, OpCodeParam param, Pos pos, double trace );

void printMessage(Message * msg);

// Destroy communication channel
void destroyIPC();

void openServerIPC(void * t);

void openClientIPC(void * t);

void closeServerIPC();

void closeClientIPC();

// Returns a Message on success, NULL on error
Message * receiveMessage(NodeType from, int key);

// Returns 0 on success, -1 on error
int sendMessage(NodeType to, Message * msg);

// This function MUST! be called for signal handling (close channel, etc)
// usage: signal(SIGINT, sigHandler)
void sigHandler();


void errorLog(char * s);



#endif
