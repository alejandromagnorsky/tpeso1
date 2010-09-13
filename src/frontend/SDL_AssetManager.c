#include "../../include/SDL_AssetManager.h"
#include "../../include/SDL_utils.h"
#include <string.h>

SDL_AssetVector * createAssetVector(int size){

	SDL_AssetVector * out = malloc(sizeof(SDL_AssetVector));

	if(size < 100)
		out->assets = malloc(size * sizeof(SDL_Asset));
	out->size = size;

	int i;
	for(i=0;i<out->size;i++){
		out->assets[i].original = NULL;
		out->assets[i].image = NULL;
		out->assets[i].name = NULL;
		out->assets[i].filename = NULL;
	}

	return out;
}

int getQtyActiveAssets(SDL_AssetVector * vector){
	int i;
	for(i=0;i<vector->size && vector->assets[i].original != NULL;i++);
	return i;
}

int getAssetIndex(SDL_AssetVector * vector, char * name){

	int i;
	// Look for asset
	for(i=0;i<vector->size && vector->assets[i].original !=NULL && strcmp(name, vector->assets[i].name);i++);

	// check last one
	if( i == vector->size && vector->assets[i].original != NULL && !strcmp(name, vector->assets[i-1].name))
		return -1;

	return  i;
}

SDL_Asset * getAssetByName(SDL_AssetVector * vector,char * name){

	int i;
	// Look for asset
	for(i=0;i<vector->size && vector->assets[i].original !=NULL && strcmp(name, vector->assets[i].name);i++);

	// check last one
	if( i == vector->size && vector->assets[i].original != NULL && !strcmp(name, vector->assets[i-1].name))
		return NULL;

	return (vector->assets) + i;
}

// Add if possible, realloc if not
void addAsset(SDL_AssetVector * vector, char * filename, char * ext,  char * name, int alpha){
	int index;

	// Get empty asset
	for(index=0; index < vector->size && vector->assets[index].image != NULL ;index++);

	// If there is no more place, alloc 10 more empty assets
	if(index == vector->size && vector->assets[index-1].image != NULL){
		vector->assets = realloc(vector->assets, (vector->size+10)*sizeof(SDL_Asset));
		vector->size+= 10;

		// And initialize new assets
		int i;
		for(i=index;i<vector->size;i++){
			vector->assets[i].original = NULL;
			vector->assets[i].image = NULL;
			vector->assets[i].name = NULL;
			vector->assets[i].filename = NULL;
		}
	}

	char * buf = malloc(sizeof(char)*256);
	sprintf(buf, "%s%s", ASSETDIR, filename);

	vector->assets[index].original = loadImageSDL( buf, ext, alpha);
	vector->assets[index].image = loadImageSDL( buf, ext, alpha);
	vector->assets[index].name = name;
	vector->assets[index].filename = buf;

	free(buf);
}

void modifyAssetImage(SDL_AssetVector * vector,int index, double rotate, double zoom ){
	if(vector == NULL || vector->assets == NULL || index >= vector->size )
		return;

	if(vector->assets[index].original != NULL){
		SDL_FreeSurface(vector->assets[index].image);
		vector->assets[index].image = rotozoomSurface(vector->assets[index].original, 0, zoom, 1);	
	}
}

SDL_Surface * getAssetImage(SDL_AssetVector * vector,char * name){
	return getAssetByName(vector,name)->image;
}

// Delete asset, realloc 
void deleteAsset(SDL_AssetVector * vector, char * name){


}


