#include <SDL.h>
#include "../include/SDL_utils.h"
#include "../include/SDL_World.h"
#include "../include/SDL_AssetManager.h"

#define SCREEN_WIDTH 644
#define SCREEN_HEIGHT 480

#define DELAY 30	// delay in ms

int cleanUp( int err )
{
     SDL_Quit();
     return err;            
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

	SDL_WM_SetCaption("Simple Ant Colony simulation", NULL);

	SDLWorld * world = getWorld(696, 1050, "assets/bg.jpg", "JPG", SDL_MapRGB( screen->format, 0, 0, 0 ) );

	initAssets(5);

	addAsset("assets/hormiga_tmp.jpg", "JPG", "Hormiga", 0);

	SDL_Surface * hormiga = getAssetImage("Hormiga");

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

		if(keystate[SDLK_DOWN])	translateCamera(world,0,-1);
		if(keystate[SDLK_UP])	translateCamera(world,0,1);
		if(keystate[SDLK_LEFT])	translateCamera(world,1,0);
		if(keystate[SDLK_RIGHT])translateCamera(world,-1,0);


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

		renderObject(screen, world, hormiga, 4, 6);
		renderObject(screen, world, hormiga, 2, 8);
		renderObject(screen, world, hormiga, 12, 15);
		renderObject(screen, world, hormiga, 6, 10);
		renderObject(screen, world, hormiga, 14, 18);
		renderObject(screen, world, hormiga, 15, 13);

		/* Update screen */
		SDL_Flip(screen);  

		/* Delay */
//		SDL_Delay( DELAY );
     }   

	endWorld(world);
    SDL_FreeSurface(screen);
    return cleanUp(0);
}
