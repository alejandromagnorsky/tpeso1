#include <SDL.h>
#include <signal.h>
#include <unistd.h>
#include <SDL_rotozoom.h>
#include "../include/SDL_utils.h"
#include "../include/SDL_AssetManager.h"
#include "../include/GameLogic.h"
#include "../include/communication.h"

#include "../include/ant.h"
#include "../include/anthill.h"
#include "../include/map.h"
#include <pthread.h>

#define SCREEN_WIDTH 644
#define SCREEN_HEIGHT 480

int cleanUp( int err )
{
	destroyIPC();
	return err;
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

	openServer(NULL);

	pthread_t mapThread;
	pthread_create(&mapThread, NULL, mapMain,NULL);

	SDL_Surface * screen = initSDL(argc, argv);
	startGame(screen);



//	endWorld(world);


	closeIPC();
	destroyIPC();

//	SDL_FreeSurface(screen);
	return cleanUp(1);
}
