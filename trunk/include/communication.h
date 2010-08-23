typedef enum { ANT, MOVE, FOOD , SHOUT, TRACE, RECEIVED, TURN } OpCode;
typedef enum { SET, GET, OK, NOT_OK } OpCodeParam1;

typedef enum { ANTHILL, ANT, MAP } nodeType;

typedef struct{
	int pidto, pidfrom;
	OpCode opCode;
	OpCodeParam1 param1;
	Pos pos;
	double trace;
} Message;

Message * createMessage(int pidto, pidfrom, OpCode opCode, OpCodeParam1 param1, Pos pos, double trace );

int connect(nodeType t, int pidto);

Message * receive(int pidfrom);

void send(Message * msg);

void close(int pidto);
