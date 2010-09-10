#include <SDL.h>
#include "SDL_utils.h" 
#include "SDL_AssetManager.h"

#define LAYERS 5

#define ANIMATED 1
#define ORIENTED 1
#define ALPHA 1

typedef enum { SPRITE_NULL, SPRITE_DOWN, SPRITE_UP, SPRITE_RIGHT, SPRITE_LEFT } Orientation; // Sprite, not Seven Up! ;)

typedef struct{
	int id;	// Asset id
	int animated;
	int oriented;
	Orientation orientation;
	int frame;
	double offsetX, offsetY;	// offset for moving between grids
} GridObject;

typedef struct {
	int sizeX,sizeY;		//size in grids
	int cameraX, cameraY;	//camera position

	double zoomFactor;
	SDL_Surface * bgimage;
	Uint32 bgcolor;
	SDL_AssetVector * vector;

	int gridSize;
	GridObject * * grid[LAYERS]; // A matrix with layers, each element represents its index in AssetVector
} SDL_World;

SDL_World * getSDLWorld(int h, int w, char * filename, char * ext, Uint32 bgcolor);

void zoom(SDL_World * world, double z);

void endWorld(SDL_World * world);	// libera recursos!! IMPORTANTE

void renderSDLWorld(SDL_World * world, SDL_Surface * screen);	// Por ahora, redibuja todo...

void translateCamera(SDL_World * world, int x, int y);

void setFrame(SDL_World * world, int x, int y, int layer, int frame);

void nextFrame(SDL_World * world, int x, int y, int layer);

// Strings may be inefficient, should change later ( soy un colgado )
void addObject(SDL_World * world, char * id, int x, int y, int layer, int animated, int oriented );

void addObjectAlpha(SDL_World * world, char * id, int x, int y, int layer, int animated, int oriented, int alpha );

// Just object, not asset
void deleteObject(SDL_World * world, int x, int y, int layer );

// Move objects in same layer
int moveObject(SDL_World * world, int fromX, int fromY, int toX, int toY, int layer);
