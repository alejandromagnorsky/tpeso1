#include <SDL.h>
#include <signal.h>
#include <unistd.h>
#include <SDL_rotozoom.h>
#include "../../include/SDL_utils.h"
#include "../../include/SDL_AssetManager.h"
#include "../../include/GameLogic.h"
#include "../../include/transport.h"

#include "../../include/ant.h"
#include "../../include/anthill.h"
#include "../../include/map.h"
#include <pthread.h>

#define SCREEN_WIDTH 644
#define SCREEN_HEIGHT 480

#define AUDIO_RATE 22050
#define AUDIO_FORMAT AUDIO_S16SYS
#define AUDIO_CHANNELS 2
#define AUDIO_BUFFERS 4096

int cleanUp( int err )
{
	closeSounds();
	destroyIPC();
	return err;
}


SDL_Surface * initSDL(int argc, char * argv[]){
	SDL_Surface * screen;

	/* Initialize video & audio, or at least try */
	if( (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO)) != 0 )
	{
		fprintf(stderr, "SDL could not be initialized: %s\n", SDL_GetError());
		exit(1);
	}

	/* Initialize video mode */
	if( (screen = SDL_SetVideoMode(SCREEN_WIDTH,
	       SCREEN_HEIGHT, 32,/* SDL_NOFRAME |*/ SDL_DOUBLEBUF | SDL_SWSURFACE | SDL_ANYFORMAT | SDL_HWPALETTE )) == NULL )
	{
		fprintf(stderr, "SDL could not set video mode: %s\n", SDL_GetError());
		cleanUp(1);
	}

	/* Initialize audio mode */
	if(Mix_OpenAudio(AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS) != 0) {
		fprintf(stderr, "SDL could not set audio mode: %s\n", Mix_GetError());
		exit(1);
	}

	SDL_WM_SetCaption("Simple Ant Colony simulation", NULL);

	return screen;
}

int main(int argc, char * argv[]){

	signal(SIGINT, sigHandler);

	SDL_Surface * screen = initSDL(argc, argv);

	// MAP LOADER HERE
	World * world;
	//world = mondoGenerator();	

	openSounds();

	world = getWorld("testmap");
	pthread_t mapThread;
	pthread_create(&mapThread, NULL, mapMain,(void *) world);

	startGame(screen, world->sizeX, world->sizeY);

//	endWorld(world);

	destroyIPC();

//	SDL_FreeSurface(screen);
	return cleanUp(1);
}
