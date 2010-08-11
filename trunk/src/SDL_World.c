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

void renderSDLWorld(SDLWorld * world, SDL_Surface * screen){

	blitSurface( screen, world->bg, 0, 0);

}

