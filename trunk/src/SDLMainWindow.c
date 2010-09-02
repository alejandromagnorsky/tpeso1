#include <SDL.h>
#include <signal.h>
#include <unistd.h>
#include <SDL_rotozoom.h>
#include "../include/SDL_utils.h"
#include "../include/SDL_AssetManager.h"
#include "../include/GameLogic.h"
#include "../include/communication.h"
#include <pthread.h>

#define SCREEN_WIDTH 644
#define SCREEN_HEIGHT 480

int cleanUp( int err )
{
	destroyIPC();
	return err;
}

void startMapEngine(){

	int frontendPID = getpid();

	int pid = fork();
	char * argPID = malloc(10*sizeof(char));

	sprintf(argPID, "%d", frontendPID);

	switch(pid){
		case -1: 
			printf("can't fork\n");
			exit(-1);	
			break;
		case 0 : // this is the code the child runs 
			execl ("./map","map",argPID, NULL);
			break;
		default: // this is the code the parent runs 
			// Get back to frontend
			break;
	}
}

SDL_Surface * initSDL(int argc, char * argv[]){
	SDL_Surface * screen;

	/* Initialize video, or at least try */
	if( (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) != 0 )
	{
		fprintf(stderr, "SDL could not be initialized: %s\n", SDL_GetError());
		exit(1);
	}

	/* Initialize video mode */
	if( (screen = SDL_SetVideoMode(SCREEN_WIDTH,
	       SCREEN_HEIGHT, 32, SDL_SWSURFACE | SDL_ANYFORMAT | SDL_HWPALETTE )) == NULL )
	{
		fprintf(stderr, "SDL could not set video mode: %s\n", SDL_GetError());
		cleanUp( 1 );
	}

	SDL_WM_SetCaption("Simple Ant Colony simulation", NULL);

	return screen;
}

int main(int argc, char * argv[]){

	signal(SIGINT, sigHandler);

	startMapEngine();

	SDL_Surface * screen = initSDL(argc, argv);

	startGame(screen);

//	endWorld(world);
	SDL_FreeSurface(screen);
	return cleanUp(1);
}
