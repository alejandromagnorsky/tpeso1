#include <SDL.h>
#include <signal.h>
#include <unistd.h>
#include <SDL_rotozoom.h>
#include "../include/SDL_utils.h"
#include "../include/SDL_World.h"
#include "../include/SDL_AssetManager.h"
#include "../include/communication.h"
#include <pthread.h>

#define SCREEN_WIDTH 644
#define SCREEN_HEIGHT 480

#define SENSITIVITY 2

#define DELAY 30	// delay in ms

int aux = 0;

int cleanUp( int err )
{
     SDL_Quit();
     return err;            
}

void * checkMessages(void * threadid){


	Message * smsg;
	Message * rmsg;

	while(1){

		smsg = NULL;
		rmsg = NULL;

		rmsg = receiveMessage(ANT);
		int ant_id = rmsg->pidFrom;

		if( rmsg != NULL){
			Pos pos = { 0, 0 };
			smsg = createMessage(getpid(), ant_id, MOVE, OK, pos, 0);
			sendMessage(ANT, smsg);
			aux = 1 - aux;
		}
	}
	pthread_exit(NULL);
}


int main(int argc, char * argv[]){

	SDL_Surface * screen;
	SDL_Event event;
	Uint8 * keystate;

	/* Initialize video, or at least try */
	if( (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) != 0 )
	{
		fprintf(stderr, "SDL could not be initialized: %s\n", SDL_GetError());
		return 1;
	}

	/* Initialize video mode */
	if( (screen = SDL_SetVideoMode(SCREEN_WIDTH,
	       SCREEN_HEIGHT, 32, SDL_SWSURFACE | SDL_ANYFORMAT | SDL_HWPALETTE )) == NULL )
	{
		fprintf(stderr, "SDL could not set video mode: %s\n", SDL_GetError());
		return cleanUp( 1 );
	}


	/* Comunicacion
	   Aca recibe donde esta el hormiguero, el tamaño del mapa, y donde esta 
	   la comida inicialmente.
	*/

	signal(SIGINT, sigHandler);

	SDL_WM_SetCaption("Simple Ant Colony simulation", NULL);

	initAssets(5);

	SDLWorld * world = getWorld(696, 1050, "assets/bg.jpg", "JPG", SDL_MapRGB( screen->format, 0, 0, 0 ) );

	addAsset("assets/hormiga_tmp.jpg", "JPG", "Hormiga", 1);

	SDL_Surface * hormiga = getAssetImage("Hormiga");

	modifyAssetImage("Hormiga", 5, 1);

	zoom(world,1);


	pthread_t thread;
	pthread_create(&thread, NULL, checkMessages, NULL);

  	while(1)
     	{
		while( SDL_PollEvent( &event ) )
                    /* See if user  quits */
                    if(event.type == SDL_QUIT )
			return cleanUp(1);
           	


		/* Get a keyboard snapshot */
		keystate = SDL_GetKeyState( NULL );

		// If user presses ESC
		if(keystate[SDLK_ESCAPE])
			return cleanUp(1);;

		if(keystate[SDLK_DOWN])	translateCamera(world,0,-SENSITIVITY);
		if(keystate[SDLK_UP])	translateCamera(world,0,SENSITIVITY);
		if(keystate[SDLK_LEFT])	translateCamera(world,SENSITIVITY,0);
		if(keystate[SDLK_RIGHT])translateCamera(world,-SENSITIVITY,0);
		if(keystate[SDLK_a]) zoom(world,1.1);
		if(keystate[SDLK_z]) zoom(world,0.9);



		/* 
		Aca este proceso deberia recibir los datos de:
			- hormigas
			- comida
			- rastros
		y nada mas, con esto ya puede mostrarlos en pantalla.
		OJO: Solo si se mueven. Estos procesos deberian avisar que se movieron.
		No tiene sentido que se redibuje la pantalla todo el tiempo...
 		*/  



		renderSDLWorld(world, screen);

		if(aux){
			renderObject(screen, world, hormiga, 4, 6);
			renderObject(screen, world, hormiga, 2, 8);
			renderObject(screen, world, hormiga, 12, 15);
			renderObject(screen, world, hormiga, 14, 18);
		}
	/* Update screen */
		SDL_Flip(screen);  

		/* Delay */
//		SDL_Delay( DELAY );
     }   

	endWorld(world);
    SDL_FreeSurface(screen);
    return cleanUp(0);
}
