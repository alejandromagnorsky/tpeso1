#include "../include/SDL_World.h"
#include "../include/SDL_utils.h"

SDLWorld * getWorld(int h, int w, char * filename, char * ext){
	SDLWorld * out = malloc(sizeof(SDLWorld));
	out->h = h;
	out->w = w;
	out->cameraX = 0;
	out->cameraY = 0;
	out->bg = loadImageSDL( filename, ext , 0);

	return out;
}

void translateCamera(SDLWorld * world, int x, int y){
	world->cameraX += x;
	world->cameraY += y;
}

void endWorld(SDLWorld * world){
	SDL_FreeSurface(world->bg);
	free(world);
}

void renderGrid(SDLWorld * world, SDL_Surface * screen, int gridSize){

	// Nota: tengo que saber la posicion de la camara, para desfasar todo.. es facil

	int i;
	int dX, dY; // camera position 

	dX = world->cameraX % gridSize;
	dY = world->cameraY % gridSize;
	Uint32 gridColor = SDL_MapRGB( screen->format, 255, 255, 255 );

	// Verticales
	for(i=0;i<(screen->w / gridSize)+1;i++)
		SDL_printLine(screen, dX+ i*gridSize, 0, dX+ i*gridSize, screen->h, gridColor );

	// Horizontales
	for(i=0;i<(screen->h / gridSize)+1;i++)
		SDL_printLine(screen, 0, dY+i*gridSize, screen->w, dY+i*gridSize, gridColor );
		
}

void renderSDLWorld(SDLWorld * world, SDL_Surface * screen){

	blitSurface( screen, world->bg, 0, 0);
	renderGrid(world,screen, 40);

}

