
// Destroy communication channel
void destroyIPC();


void openServer(void * t, int size);

void closeServer();


void openClient(void * t, int size);

void closeClient();


// Key is client key
int receiveFromServer( int key, char * buf, int size );

int sendToClient( int key, char * buf, int size );


int receiveFromClient( int serverKey, char * buf, int size );

int sendToServer( int serverKey, char * buf, int size );

// This function MUST! be called for signal handling (close channel, etc)
// usage: signal(SIGINT, sigHandler)
void sigHandler();
