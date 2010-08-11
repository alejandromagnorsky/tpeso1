#include "../include/SDL_World.h"
#include "../include/SDL_utils.h"

SDLWorld * getWorld(int h, int w, char * filename, char * ext){
	SDLWorld * out = malloc(sizeof(SDLWorld));
	out->h = h;
	out->w = w;
	out->bg = loadImageSDL( filename, ext , 0);

	return out;
}

void endWorld(SDLWorld * world){
	SDL_FreeSurface(world->bg);
	free(world);
}

void renderGrid(SDLWorld * world, SDL_Surface * screen, int gridSize){
	int i;

	Uint32 gridColor = SDL_MapRGB( screen->format, 255, 255, 255 );

	for(i=0;i<(screen->w / gridSize)+1;i++)
		SDL_printLine(screen, i*gridSize, 0, i*gridSize, screen->h, gridColor );
	for(i=0;i<(screen->h / gridSize)+1;i++)
		SDL_printLine(screen, 0, i*gridSize, screen->w, i*gridSize, gridColor );
		
}

void renderSDLWorld(SDLWorld * world, SDL_Surface * screen){

	blitSurface( screen, world->bg, 0, 0);
	renderGrid(world,screen, 50);

}

