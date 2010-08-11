#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>


SDL_Surface * loadImageSDL( char * name, char * type, int isAlpha);

void blitSurface(  SDL_Surface * dst, SDL_Surface *src, int x, int y);

void blitSurfaceEndlessBorder(  SDL_Surface * dst, SDL_Surface *src, int x, int y, int circularBorder);

/* Blit animated surface */
void blitAnim( SDL_Surface * dst, SDL_Surface * src, int rectW, int rectH, int frame, int x, int y );

void SDL_printLine(SDL_Surface * screen, int x1, int y1, int x2, int y2, Uint32 color );

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
