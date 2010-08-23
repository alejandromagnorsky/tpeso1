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

Message * createMessage(int pidTo,int pidFrom, OpCode opCode, OpCodeParam param, Pos pos, double trace );

int connect(NodeType t, int pidTo);

Message * receive(int pidFrom);

void send(Message * msg);

void close(int pidTo);
