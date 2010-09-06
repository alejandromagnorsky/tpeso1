#include "common.h"

// Destroy communication channel
void destroyIPC();


void openServer(void * t);

void openClient(void * t);

// Open & initialize IPC resource
void openIPC();

// Close IPC resource
void closeIPC();


// Returns a Message on success, NULL on error
Message * receiveMessage(NodeType from, int key);

// Returns 0 on success, -1 on error
int sendMessage(NodeType to, Message * msg);

// This function MUST! be called for signal handling (close channel, etc)
// usage: signal(SIGINT, sigHandler)
void sigHandler();
