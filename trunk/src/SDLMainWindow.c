#include <SDL.h>
#include "../include/SDL_utils.h"
#include "../include/SDL_World.h"


#define SCREEN_WIDTH 1050
#define SCREEN_HEIGHT 696

#define DELAY 30	// delay in ms

int cleanUp( int err )
{
     SDL_Quit();
     return err;            
}


int main(int argc, char * argv[]){

	SDL_Surface * screen;
	SDL_Event event;

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


	SDLWorld * world = getWorld(696, 1050, "assets/bg.jpg", "JPG" );

  	while(1)
     	{
		while( SDL_PollEvent( &event ) )
           	{
                    /* See if user presses ESC or quits */
                    if(event.type == SDL_QUIT )
			return cleanUp(1);
		    else if( event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_ESCAPE)
                                            return cleanUp(1);
           	}


		/* 
		Aca este proceso deberia recibir los datos de:
			- hormigas
			- comida
			- rastros
		y nada mas, con esto ya puede mostrarlos en pantalla.
		OJO: Solo si se mueven. Estos procesos deberian avisar que se movieron.
 		*/  

		// render World();

		renderSDLWorld(world, screen);

		/* Update screen */
		SDL_Flip(screen);  

		/* Delay */
//		SDL_Delay( DELAY );
     }   

    SDL_FreeSurface(screen);
    return cleanUp(0);
}
