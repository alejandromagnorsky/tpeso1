#include <SDL.h>
#include "SDL_utils.h" 

typedef struct {
	int h,w;		//size
	int cameraX, cameraY;	//camera position
	int gridSize;
	SDL_Surface * bgimage;
	SDL_Rect bg;
	Uint32 bgcolor;
} SDLWorld;



SDLWorld * getWorld(int h, int w, char * filename, char * ext, Uint32 bgcolor);

void endWorld(SDLWorld * world);	// libera recursos!! IMPORTANTE

void renderSDLWorld(SDLWorld * world, SDL_Surface * screen);	// Por ahora, redibuja todo...

void translateCamera(SDLWorld * world, int x, int y);


// These update functions are used to update ants,traces and foods' positions
// on map, not having to redraw the whole map each frame.

void updateAnt();

void updateTrace();

void updateFood();



void renderObject(SDL_Surface *screen, SDLWorld * world, SDL_Surface * s, int xGrid, int yGrid);
