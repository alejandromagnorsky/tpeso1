#include <SDL.h>

#ifndef SDL_ASSETMANAGER
#define SDL_ASSETMANAGER

typedef struct{
	SDL_Surface * image;
	SDL_Surface * original;
	char * name;
	char * filename;
} SDL_Asset;

typedef struct{
	SDL_Asset * assets;
	int size;
} SDL_AssetVector;

// Set init qty of assets
SDL_AssetVector * createAssetVector(int size);

int getAssetIndex(SDL_AssetVector * vector, char * name);

SDL_Asset * getAssetByName(SDL_AssetVector * vector, char * name);

int getQtyActiveAssets(SDL_AssetVector * vector);

void modifyAssetImage(SDL_AssetVector * vector , char * name, double rotate, double zoom );

// Add if possible, realloc if not
void addAsset(SDL_AssetVector * vector, char * filename, char * ext, char * name, int alpha);

// Delete asset, realloc 
void deleteAsset(SDL_AssetVector * vector, char * name);

// Get asset by name
SDL_Surface * getAssetImage(SDL_AssetVector * vector,char * name);

#endif 
