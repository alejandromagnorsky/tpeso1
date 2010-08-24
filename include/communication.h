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

Message * createMessage(int pidTo, int pidFrom, OpCode opCode, OpCodeParam param, Pos pos, double trace );

void closeNode(NodeType t);
Message * receiveMessage(NodeType from);

// Returns 0 on success, -1 on error
int sendMessage(NodeType to, Message * msg);

