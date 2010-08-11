#include "../include/SDL_utils.h"

/* Load an optimized image */
SDL_Surface * loadImageSDL( char * name, char * type, int isAlpha)
{
           SDL_Surface *     image;
           SDL_Surface * optimized;

           /* Use SDL_image to load image type */
           image = IMG_LoadTyped_RW(SDL_RWFromFile(name, "rb"), 1, type);

           /* Optimize the surface through DisplayFormat, and check if it
              has alpha channel */
           if(isAlpha) optimized = SDL_DisplayFormatAlpha( image );
           else optimized = SDL_DisplayFormat( image );
           
           SDL_FreeSurface( image );

           return optimized;
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
           if( printAux[i] )
           SDL_BlitSurface( src, NULL, dst, &aux[i]);   
}


/* Blit surface but animated */
void blitAnim( SDL_Surface * dst, SDL_Surface * src, int rectW, int rectH, int frame, int x, int y )
{
            SDL_Rect spriteRect;
            
            spriteRect.w = rectW;
            spriteRect.h = rectH;
            spriteRect.y = 0;
            spriteRect.x = (frame-1) * rectW;
            

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
           

           SDL_BlitSurface( src, &spriteRect, dst, &rect);   
           
           for(i=0;i<4;i++)
           if( printAux[i] )
           SDL_BlitSurface( src, &spriteRect, dst, &aux[i]);           
}



