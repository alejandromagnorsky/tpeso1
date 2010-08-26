#include "common.h"

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
