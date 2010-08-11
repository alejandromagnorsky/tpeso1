#include "../include/SDL_World.h"
#include "../include/SDL_utils.h"

SDLWorld * getWorld(int h, int w, char * filename, char * ext, Uint32 bgcolor){
	SDLWorld * out = malloc(sizeof(SDLWorld));
	out->h = h;
	out->w = w;
	out->cameraX = 0;
	out->cameraY = 0;
	out->gridSize = 40;
	out->bgimage = loadImageSDL( filename, ext , 0);

	out->bg.x = 0;
	out->bg.y = 0;
	out->bg.h = h;
	out->bg.w = w;

	out->bgcolor = bgcolor;

	return out;
}

void translateCamera(SDLWorld * world, int x, int y){
	world->cameraX += x;
	world->cameraY += y;
}

void endWorld(SDLWorld * world){
	SDL_FreeSurface(world->bgimage);
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


	// Clear map
	 SDL_FillRect( screen, &world->bg, world->bgcolor);         

	// Ver bien el tema del tiling, para evitar los artifacts
	// Esta es una solucion barata para que se mueva, y nada mas
	blitSurface( screen, world->bgimage, world->cameraX % world->bgimage->w, world->cameraY % world->bgimage->h);
	renderGrid(world,screen, world->gridSize);

}

void renderObject(SDL_Surface * screen, SDLWorld * world, SDL_Surface * s, int xGrid, int yGrid){

	int x = world->cameraX + xGrid*world->gridSize;
	int y = world->cameraY + yGrid*world->gridSize;

	blitSurface(screen, s, x,y);

}
