#include "../include/SDL_World.h"
#include "../include/SDL_utils.h"
#include "../include/SDL_AssetManager.h"

SDL_World * getSDLWorld(int sizeX, int sizeY, char * filename, char * ext, Uint32 bgcolor){
	SDL_World * out = malloc(sizeof(SDL_World));
	out->sizeX = sizeX;
	out->sizeY = sizeY;
	out->cameraX = 0;
	out->cameraY = 0;
	out->gridSize = 50;
	out->zoomFactor = 1;

	out->vector = createAssetVector(10);

	addAsset(out->vector, filename, ext, "World Background", 0);
	out->bgimage = getAssetImage(out->vector, "World Background");
	out->bgcolor = bgcolor;

	//printf("%d, %d \n", sizeX, sizeY);

	int i,j,k;
	for(i=0;i<LAYERS;i++)
		out->grid[i] = calloc(sizeX,sizeof(GridObject *));

	for(i=0;i<LAYERS;i++)
		for(j=0;j<sizeX;j++)
			out->grid[i][j] = calloc(sizeY,sizeof(GridObject));

	GridObject empty;
	empty.id = -1;
	empty.frame = 0;
	empty.offsetX = 0;
	empty.offsetY = 0;
	empty.animated = !ANIMATED;
	empty.oriented = !ORIENTED;
	empty.orientation = SPRITE_DOWN;

	for(i=0;i<LAYERS;i++)
		for(j=0;j<sizeX;j++)
			for(k=0;k<sizeY;k++)
				out->grid[i][j][k] = empty;
	return out;
}

void translateCamera(SDL_World * world, int x, int y){
	world->cameraX += x;
	world->cameraY += y;
}

void endWorld(SDL_World * world){
	SDL_FreeSurface(world->bgimage);
	free(world);
}

void zoom(SDL_World * world, double z){
	if(z * world->zoomFactor < 0.1 )
		return;

	world->zoomFactor = ( z * world->zoomFactor >= 1) ? 1 : world->zoomFactor * z;
	int qty = getQtyActiveAssets(world->vector);
	int i;
	for(i=0;i<qty;i++)
		modifyAssetImage(world->vector, world->vector->assets[i].name, 0, world->zoomFactor);
}

void renderGrid(SDL_World * world, SDL_Surface * screen, int gridSize){

	// Nota: tengo que saber la posicion de la camara, para desfasar todo.. es facil

	int i;

	Uint32 gridColor = SDL_MapRGB( screen->format, 255, 255, 255 );

	// Verticales
	for(i=0;i<world->sizeX+1;i++)
		SDL_printLine(screen, world->cameraX + i*gridSize * world->zoomFactor, world->cameraY, world->cameraX + i*gridSize * world->zoomFactor, world->sizeY*world->gridSize*world->zoomFactor + world->cameraY, gridColor );

	// Horizontales
	for(i=0;i<world->sizeY+1;i++)
		SDL_printLine(screen, world->cameraX , world->cameraY + i*gridSize * world->zoomFactor, world->sizeX*world->gridSize*world->zoomFactor + world->cameraX, world->cameraY + i*gridSize * world->zoomFactor, gridColor );
}

void renderSDLWorld(SDL_World * world, SDL_Surface * screen){

	SDL_Rect bg;

	bg.x = 0;
	bg.y = 0;
	bg.h = screen->h;
	bg.w = screen->w;

	// Clear map
	 SDL_FillRect( screen, &bg, world->bgcolor);         

	// Esta es una solucion barata para que se mueva, y nada mas
	blitSurface( screen, world->bgimage, world->cameraX , world->cameraY);
	renderGrid(world,screen, world->gridSize);

	int i,j,k;

	for(i=0;i<LAYERS;i++)
		for(j=0;j<world->sizeX;j++)
			for(k=0;k<world->sizeY;k++){

				GridObject grid = world->grid[i][j][k];
				int index = grid.id;	

				if(index >= 0){
					int x = world->cameraX + j*world->gridSize * world->zoomFactor;
					int y = world->cameraY + k*world->gridSize * world->zoomFactor;
			
					x += grid.offsetX * world->gridSize * world->zoomFactor;
					y += grid.offsetY * world->gridSize * world->zoomFactor;

					int blitRect = world->gridSize * world->zoomFactor;

					int centered = blitRect /2 ;

					if(grid.animated)
						blitAnim( screen, world->vector->assets[index].image, blitRect, blitRect, grid.orientation, grid.frame, x, y);
					else
						blitSurfaceCentered(screen, world->vector->assets[index].image, x + centered, y + centered);	
					
				}
			}
}

int verifyGrid(SDL_World * world, int x, int y, int layer){
	if( x >= 0 && x < world->sizeX && y >= 0 && y < world->sizeY && layer < LAYERS )
		return 1;
	return 0;
}

void nextFrame(SDL_World * world, int x, int y, int layer) {
	setFrame(world,x,y,layer, world->grid[layer][x][y].frame + 1);
}


void setFrame(SDL_World * world, int x, int y, int layer, int frame) {
	int index = world->grid[layer][x][y].id;

	if( index == -1 ) 
		return;

	printf("Orientation:%d\n", world->grid[layer][x][y].orientation);

	int frames;
	// Only works for animated assets (or assets that have frames)
	if(world->grid[layer][x][y].animated){
		if(world->grid[layer][x][y].oriented)	// If oriented, h is 4 times larger, for orientation sprites 
			frames = world->vector->assets[index].image->w / (world->vector->assets[index].image->h/4);
		else					
			frames = world->vector->assets[index].image->w / world->vector->assets[index].image->h;
	}

	printf("max frames:%d intended frame: %d \n", frames, frame);
	
	if( frame > frames ){
		world->grid[layer][x][y].frame = 0;
	} else world->grid[layer][x][y].frame = frame;

}

void addObject(SDL_World * world, char * id, int x, int y, int layer, int animated, int oriented){
	int index = getAssetIndex( world->vector, id);

	// If asset exists and position is valid
	if( index >= 0 && verifyGrid(world, x, y, layer))
		if(world->grid[layer][x][y].id == -1){	// Grid must be empty
			world->grid[layer][x][y].id = index;
			world->grid[layer][x][y].animated = animated;
			world->grid[layer][x][y].frame = 1;
			world->grid[layer][x][y].oriented = oriented;
			world->grid[layer][x][y].orientation = SPRITE_DOWN; // Down is default
			world->grid[layer][x][y].offsetX = 0;
			world->grid[layer][x][y].offsetY = 0;
		}
}

// Just object, not asset
void deleteObject(SDL_World * world, int x, int y, int layer ){
	if(verifyGrid(world,x,y,layer))
		world->grid[layer][x][y].id = -1;
}

// Move objects in same layer
// Returns 1 if finished moving
// Returns 0 if still waiting to move
int moveObject(SDL_World * world, int fromX, int fromY, int toX, int toY, int layer){
	int index = world->grid[layer][fromX][fromY].id;

	// If object doesn't exist, finish
	if(index <0)
		return 1;

	// If asset exists and position is valid
	if( verifyGrid(world, fromX, fromY, layer) && verifyGrid(world, toX, toY, layer)
		&& ( fromX != toX || fromY != toY )){

		// Still on fromX,fromY.. moving with offset
		GridObject * nextGrid = &world->grid[layer][toX][toY];	
		GridObject * oldGrid = &world->grid[layer][fromX][fromY];	

		int dX = toX - fromX;
		int dY = toY - fromY;

		int * dG; // Generic derivative
		double * offset; // Generic offset

		// Move must be done in one direction, by one unit
		if( (dX == 0 || dY == 0) && (abs(dX) == 1 || abs(dY) == 1) ){

			// Guess frames
			int frames;
			int orientation;
			if(oldGrid->animated){
				if(oldGrid->oriented)	// If oriented, h is 4 times larger, for orientation sprites 
					frames = world->vector->assets[index].image->w / (world->vector->assets[index].image->h/4);
				else					
					frames = world->vector->assets[index].image->w / world->vector->assets[index].image->h;
			} else frames = 5;	// default movement 

			// Lets guess what to move
			if(abs(dX) == 1){
				dG = &dX;	
				offset = &(oldGrid->offsetX);
				orientation = dX > 0 ? SPRITE_LEFT : SPRITE_RIGHT;

			} else if( abs(dY) == 1){
				dG = &dY;
				offset = &(oldGrid->offsetY);
				orientation = dY > 0 ? SPRITE_UP : SPRITE_DOWN;
			}

			if(oldGrid->oriented)
				oldGrid->orientation = orientation;

			// Move until got to next grid
			if( (*offset) <= abs(*dG) && oldGrid->frame <= frames){
				(*offset) += (*dG) / (double)frames;
				oldGrid->frame++;
			}

			// When finally moved, change grid values
			if(fabs((*offset) - (*dG)) <= 0.1 ){
				nextGrid->id = index;
				nextGrid->offsetX = 0;
				nextGrid->offsetY = 0;
				nextGrid->frame = 1;
				nextGrid->animated = oldGrid->animated;
				nextGrid->oriented = oldGrid->oriented;
				nextGrid->orientation = oldGrid->orientation;

				oldGrid->id = -1;
				(*offset) = 0;
				oldGrid->frame = 1;
				oldGrid->animated = !ANIMATED;
				oldGrid->oriented = !ORIENTED;
				oldGrid->orientation = SPRITE_DOWN; // Down is default

				return 1; // finished moving
			}
		}
	}

	return 0;	// still moving
}
