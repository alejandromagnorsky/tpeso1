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
	pthread_t * antThreads = malloc(sizeof(pthread_t)*ants);
	for(i=0;i<ants;i++)
     		pthread_create(&antThreads[i], NULL, antMain, (void *)(i+1+key));

	int smallPoints,bigPoints;
	smallPoints = 0;
	bigPoints = 0;
	
	// Esperar a recibir comida
	Message * received;
	while(1){
		received = receiveMessage(SERVER,ANTHILL_KEY);
		if(received->opCode == FOOD ){
			if(received->param == SET)
				smallPoints++;
			else if(received->param == BIG)
				bigPoints+=5;
		}
		deleteMessage(received);
	}

	closeClientIPC();

	return 0;
}

