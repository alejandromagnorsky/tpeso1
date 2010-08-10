#include <SDL.h>
#include "sdl_aux.h"


// gcc sdl_test.c sdl_aux.c -Iinclude/SDL -Llib -lSDL -lSDL_image -g -Wall


#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define DELAY 30	// delay in ms

int cleanUp( int err )
{
     SDL_Quit();
     return err;            
}


int main(int argc, char * argv[]){

	SDL_Surface * screen;
	SDL_Event  event;
	SDL_Surface * bg;

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


	bg = (SDL_Surface *)loadImageSDL( "prueba_high.jpg", "JPG", 1);

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

		blitSurface( screen, bg, 0,0);

		/* Update screen */
		SDL_Flip(screen);  

		/* Delay */
//		SDL_Delay( DELAY );
     }   

    SDL_FreeSurface(screen);
    return cleanUp(0);
}
