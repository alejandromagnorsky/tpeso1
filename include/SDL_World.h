#include <SDL.h>
#include "SDL_utils.h" 
#include "SDL_AssetManager.h"

typedef struct {
	int h,w;		//size
	int cameraX, cameraY;	//camera position
	double cameraRotation;
	int gridSize;
	double zoomFactor;
	SDL_Surface * bgimage;
	SDL_Rect bg;
	Uint32 bgcolor;
	SDL_AssetVector * vector;

	int * * * objects; // A matrix with layers, each element represents its index in AssetVector
} SDLWorld;

SDLWorld * getWorld(int h, int w, char * filename, char * ext, Uint32 bgcolor);

void zoom(SDLWorld * world, double z);

void endWorld(SDLWorld * world);	// libera recursos!! IMPORTANTE

void renderSDLWorld(SDLWorld * world, SDL_Surface * screen);	// Por ahora, redibuja todo...

void translateCamera(SDLWorld * world, int x, int y);

void renderObject(SDL_Surface *screen, SDLWorld * world, SDL_Surface * s, int xGrid, int yGrid);
