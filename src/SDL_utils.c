#include "../include/SDL_utils.h"
#include <math.h>

/* Load an optimized image */
SDL_Surface * loadImageSDL( char * name, char * type, int isAlpha)
{
	SDL_Surface *     image;
	SDL_Surface * optimized;

	/* Use SDL_image to load image type */
	image = IMG_LoadTyped_RW(SDL_RWFromFile(name, "rb"), 1, type);

	/* Optimize the surface through DisplayFormat, and check if it
	has alpha channel */
	if(isAlpha){
		SDL_SetColorKey(image,SDL_RLEACCEL, 0);
		optimized = SDL_DisplayFormatAlpha( image );
	}			
	else optimized = SDL_DisplayFormat( image );

	SDL_FreeSurface( image );

	return optimized;
}

/*
   Blit with Zoom
   WARNING: TOOOO SLOW =(
*/
void blitSurfaceZoom(  SDL_Surface * dst, SDL_Surface *src, int x, int y, double zoom){
	SDL_Surface * tmp = rotozoomSurface(src, 0, zoom, 1);
	blitSurface(dst, tmp, x, y);
	SDL_FreeSurface(tmp);
}

/* Function blitSurface.
   It blits a surface 
   onto another surface.
   Parameters:
              SDL_Surface * dst        - where to blit
              SDL_Surface *src         - what to blit
              int x, int y             - X,Y position
*/
void blitSurface(  SDL_Surface * dst, SDL_Surface *src, int x, int y)
{
	blitSurfaceEndlessBorder(dst, src, x, y, 0);
}

void blitSurfaceCentered(SDL_Surface * dst, SDL_Surface *src, int x, int y){

	if( dst == NULL)
		return;

	SDL_Rect rect;

	rect.x = x - src->w /2;
	rect.y = y - src->h /2;
	rect.w = src->w;
	rect.h = src->h;

	SDL_BlitSurface( src, NULL, dst, &rect);   
}


/* Function blitSurface.
   It blits a surface 
   onto another surface.
   Parameters:
              SDL_Surface * dst        - where to blit
              SDL_Surface *src         - what to blit
              int x, int y             - X,Y position
*/
void blitSurfaceEndlessBorder(  SDL_Surface * dst, SDL_Surface *src, int x, int y, int circularBorder)
{

	   if( dst == NULL)
		return;

           SDL_Rect rect;
           SDL_Rect aux[4];
           
           int i, dH, dW, printAux[4] = {0};
           
           /* Initialize aux and rect */
           for(i=0;i<4;i++){
		      aux[i].x = x;
		      aux[i].y = y;
		      aux[i].w = src->w;
		      aux[i].h = src->h;
           }
         
           rect.x = x;
           rect.y = y;
           rect.w = src->w;
           rect.h = src->h;
           
           /* Check boundaries of surface, and blit them properly */  
           if( src->h + y > dst->h && y < dst->h ) {
               dH = dst->h - y;
               aux[0].y = -dH;
               printAux[0] = 1;
           } 
           
           if( y < 0 && y + src->h > 0) {
               dH = 0 - y;
               aux[1].y = dst->h - dH;
               printAux[1] = 1;
           } 
           
           if( x < 0 && x + src->w > 0 ) {
               dW = 0 - x;
               aux[2].x = dst->w - dW;
               printAux[2] = 1;
           } 
         
           if( src->w + x > dst->w && x < dst->w ) {
               dW = dst->w - x;
               aux[3].x = -dW;
               printAux[3] = 1;
           } 
           

           SDL_BlitSurface( src, NULL, dst, &rect);   
           
           for(i=0;i<4;i++)
           if( printAux[i] && circularBorder )
           	SDL_BlitSurface( src, NULL, dst, &aux[i]);   
}


/* Blit surface but animated */
void blitAnim( SDL_Surface * dst, SDL_Surface * src, int spriteW, int spriteH, int frameY, int frameX, int x, int y )
{
	SDL_Rect spriteRect;

	spriteRect.w = spriteW;
	spriteRect.h = spriteH;
	spriteRect.y = (frameY-1) * spriteH;
	spriteRect.x = (frameX-1) * spriteW;

	if(src->w < spriteRect.x + spriteW || src->h < spriteRect.y + spriteH ){
	//	printf("Bad sprite: frameY:%d frameX:%d\n", frameY, frameX);
		return;
	}

	SDL_Rect rect;

	rect.x = x;
	rect.y = y;
	rect.w = spriteW;
	rect.h = spriteH;

	SDL_BlitSurface( src, &spriteRect, dst, &rect);   
}



/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}


void SDL_printLine(SDL_Surface * screen, int x1, int y1, int x2, int y2, Uint32 color )
{
	int x,y;
	float lambda;              
	int mX,mY;
	double distance;

	mX = x2 - x1;
	mY = y2 - y1;

	distance =  sqrt( pow( x2 - x1, 2 ) + pow( y2 - y1, 2 ) );

	if(distance == 0 ) 
		return;

	SDL_LockSurface(screen);

	for( lambda=0;lambda<1;lambda+=1/distance)
	{
	      x = x1 + lambda*mX;
	      y = y1 + lambda*mY;
	      if( x> 0 && x<screen->w && y>0 && y<screen->h) 
		putpixel(screen, x, y, color);
	}

	SDL_UnlockSurface(screen);
}
