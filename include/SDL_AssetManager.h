#include <SDL.h>

#ifndef SDL_ASSETMANAGER
#define SDL_ASSETMANAGER

typedef struct{
	SDL_Surface * image;
	char * name;
	char * filename;
} SDL_Asset;

SDL_Asset* assets;

int qtyAssets;

// Set init qty of assets
void initAssets(int init);

// Add if possible, realloc if not
void addAsset(char * filename, char * ext, char * name, int alpha);

// Delete asset, realloc 
void deleteAsset(char * name);

// Get asset by name
SDL_Surface * getAssetImage(char * name);

#endif 
