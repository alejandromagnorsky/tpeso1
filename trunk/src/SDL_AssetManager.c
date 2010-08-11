#include "../include/SDL_AssetManager.h"
#include "../include/SDL_utils.h"
#include <string.h>

void initAssets(int init){

	if(init < 100)
		assets = malloc(init * sizeof(SDL_Asset));
	qtyAssets = init;

	int i;
	for(i=0;i<qtyAssets;i++){
		assets[i].image = NULL;
		assets[i].name = NULL;
		assets[i].filename = NULL;
	}
}

// Add if possible, realloc if not
void addAsset(char * filename, char * ext,  char * name, int alpha){
	int index;
	// Get empty asset
	for(index=0; index < qtyAssets && assets[index].image != NULL ;index++);

	// If there is no more place, alloc 10 more empty assets
	if(index == qtyAssets && assets[index-1].image != NULL){
		assets = realloc(assets, (qtyAssets+10)*sizeof(SDL_Asset));
	}

	qtyAssets++;
	assets[index].image = loadImageSDL( filename, ext , alpha);
	assets[index].name = name;
	assets[index].filename = filename;

}

SDL_Surface * getAssetImage(char * name){


	int i;
	// Look for asset
	for(i=0;i<qtyAssets && assets[i].image !=NULL && !strcmp(name, assets[i].name);i++);

	// check last one
	if( i == qtyAssets && assets[i].image != NULL && !strcmp(name, assets[i-1].name))
		return NULL;

	printf("Image: %p, Name: %s, Filename: %s \n", assets[i-1].image, assets[i-1].name, assets[i-1].filename);

	return assets[i-1].image;
}

// Delete asset, realloc 
void deleteAsset(char * name){


}


