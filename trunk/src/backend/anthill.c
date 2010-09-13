#include "../../include/communication.h"
#include "../../include/transport.h"
#include "../../include/ant.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

Scream * screams;
extern int antsQuantity;

int main(int argc, char * argv[]){

	int ants, key,i;

	sscanf(argv[1], "%d", &key);
	sscanf(argv[2], "%d", &ants);
	//	printf("Anthill Key: %d Ants: %d\n", key, ants);

	openClientIPC((void*)ants);

	antsQuantity = ants;
	screams = malloc(ants*sizeof(Scream));
	for(i = 0; i < ants; i++)
		screams[i].intensity = -1;

	// Create ants, with antKey > anthillKey
	pthread_attr_t attr;

	/* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	pthread_t * antThreads = malloc(sizeof(pthread_t)*ants);
	for(i=0;i<ants;i++)
	 		pthread_create(&antThreads[i], &attr, antMain, (void *)(i+1+key));

	int smallPoints,bigPoints;
	smallPoints = 0;
	bigPoints = 0;

	// Esperar a recibir comida
	Message * received;
	while(1){
		received = receiveMessage(SERVER,ANTHILL_KEY);

		// If simulation has ended
		if(received->opCode == TURN && received->param == EMPTY)
			break;

		if(received->opCode == FOOD ){
			if(received->param == SET)
				smallPoints++;
			else if(received->param == BIG)
				bigPoints+=5;
		}
		deleteMessage(received);
	}

	void * tmp;
	for(i=0;i<ants;i++)
		pthread_join(antThreads[i], &tmp);

	printf("Hormiguero muriendo\n");
	Pos p = { 0,0};
	Message * send = createMessage(key, MAP_ID, EXIT, OK, p, 0);
	sendMessage(SERVER, send);
	deleteMessage(send);

	closeClientIPC();

	return 0;
}

