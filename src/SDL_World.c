#include "../include/SDL_World.h"
#include "../include/SDL_utils.h"
#include "../include/SDL_AssetManager.h"

SDLWorld * getWorld(int h, int w, char * filename, char * ext, Uint32 bgcolor){
	SDLWorld * out = malloc(sizeof(SDLWorld));
	out->h = h;
	out->w = w;
	out->cameraX = 0;
	out->cameraY = 0;
	out->cameraRotation = 0;
	out->gridSize = 40;
	out->zoomFactor = 1;

	addAsset(filename, ext, "World Background", 0);
	out->bgimage = getAssetImage("World Background");

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

void zoom(SDLWorld * world, double z){
	if(z *world->zoomFactor < 0.3)
		return;

	world->zoomFactor *= z;
	SDL_Asset * allAssets = getAssets();
	int qty = getQtyActiveAssets();
	int i;
	for(i=0;i<qty;i++)
		modifyAssetImage(allAssets[i].name, 0, world->zoomFactor);
}


void renderGrid(SDLWorld * world, SDL_Surface * screen, int gridSize){

	// Nota: tengo que saber la posicion de la camara, para desfasar todo.. es facil

	int i;
	int dX, dY; // camera position 

	dX = world->cameraX % (int)( gridSize * world->zoomFactor);
	dY = world->cameraY % (int)(gridSize * world->zoomFactor);
	Uint32 gridColor = SDL_MapRGB( screen->format, 255, 255, 255 );

	// Verticales
	for(i=0;i<(screen->w / (gridSize * world->zoomFactor))+1;i++)
		SDL_printLine(screen, dX+ i*gridSize * world->zoomFactor, 0, dX+ i*gridSize* world->zoomFactor, screen->h, gridColor );

	// Horizontales
	for(i=0;i<(screen->h / (gridSize * world->zoomFactor))+1;i++)
		SDL_printLine(screen, 0, dY+i*gridSize* world->zoomFactor, screen->w, dY+i*gridSize* world->zoomFactor, gridColor );
}

void renderSDLWorld(SDLWorld * world, SDL_Surface * screen){

	// Clear map
	 SDL_FillRect( screen, &world->bg, world->bgcolor);         

	// Esta es una solucion barata para que se mueva, y nada mas
	blitSurface( screen, world->bgimage, world->cameraX , world->cameraY);
	renderGrid(world,screen, world->gridSize);

}

void renderObject(SDL_Surface * screen, SDLWorld * world, SDL_Surface * s, int xGrid, int yGrid){

	int x = world->cameraX + xGrid*world->gridSize * world->zoomFactor;
	int y = world->cameraY + yGrid*world->gridSize * world->zoomFactor;

	blitSurface(screen, s, x,y);

}
