#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>


SDL_Surface * loadImageSDL( char * name, char * type, int isAlpha);

void blitSurface(  SDL_Surface * dst, SDL_Surface *src, int x, int y);

/* Blit animated surface */
void blitAnim( SDL_Surface * dst, SDL_Surface * src, int rectW, int rectH, int frame, int x, int y );
