#include "../include/common.h"
#include "../include/communication.h"
#include "../include/ant.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void * anthillMain(void * arg){

	PThreadArg parg = *((PThreadArg *)arg);

	int ants = *((int *) parg.args);
	int anthillKey = parg.key;

	printf("anthillKey: %d Ants: %d\n", anthillKey, ants);

	// Create ants, with antKey > anthillKey
	pthread_t * antThreads = malloc(sizeof(pthread_t)*ants);

	PThreadArg * antArg = malloc(sizeof(PThreadArg));
	int i;
	for(i=0;i<ants;i++){
		antArg->key = i + 1 + anthillKey;
     		pthread_create(&antThreads[i], NULL, antMain, (void *)(i+1+anthillKey));
	}


	// Esperar a recibir comida

	pthread_exit(NULL);
}

