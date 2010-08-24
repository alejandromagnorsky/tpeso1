#include "common.h"

typedef enum { REGISTER, NEIGHBOR, MOVE, FOOD , SHOUT, TRACE, RECEIVED, TURN } OpCode;

typedef enum { SET, GET, OK, NOT_OK } OpCodeParam;

typedef enum { ANTHILL, ANT, MAP } NodeType;

typedef struct{
	int pidTo, pidFrom;
	OpCode opCode;
	OpCodeParam param;
	Pos pos;
	double trace;
} Message;


// Message constructor
Message * createMessage(int pidFrom, int pidTo, OpCode opCode, OpCodeParam param, Pos pos, double trace );

// Closes a Node and its communication channel
void closeNode(NodeType t);

// Returns a Message on success, NULL on error
Message * receiveMessage(NodeType from);

// Returns 0 on success, -1 on error
int sendMessage(NodeType to, Message * msg);

// This function MUST! be called for signal handling (close channel, etc)
// usage: signal(SIGINT, sigHandler)
// No vendria mal un broadcast del server diciendo "ME CIERRO" asi los demas dejan de preguntar, no?
void sigHandler();
