#include "../include/common.h"
#include "../include/communication.h"
#include "../include/ant.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int main(int argc, char * argv[]){

	int ants, key,i;

	sscanf(argv[1], "%d", &key);
	sscanf(argv[2], "%d", &ants);
	printf("Anthill Key: %d Ants: %d\n", key, ants);

	openClient((void*)ants);

	// Create ants, with antKey > anthillKey
	pthread_t * antThreads = malloc(sizeof(pthread_t)*ants);

	for(i=0;i<ants;i++)
     		pthread_create(&antThreads[i], NULL, antMain, (void *)(i+1+key));
	
	// Esperar a recibir comida
	while(1);

	closeIPC();

	return 0;
}

