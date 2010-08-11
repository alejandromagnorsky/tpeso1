#include <SDL.h>
#include "SDL_utils.h" 

typedef struct {
	int h,w;	//size
	SDL_Surface * bg;

} SDLWorld;



SDLWorld * getWorld(int h, int w, char * filename, char * ext);

void endWorld(SDLWorld * world);	// libera recursos!! IMPORTANTE

void renderSDLWorld(SDLWorld * world, SDL_Surface * screen);	// Por ahora, redibuja todo...


// These update functions are used to update ants,traces and foods' positions
// on map, not having to redraw the whole map each frame.

void updateAnt();

void updateTrace();

void updateFood();


