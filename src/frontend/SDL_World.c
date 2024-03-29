#include "../../include/SDL_World.h"
#include "../../include/SDL_utils.h"
#include "../../include/SDL_AssetManager.h"

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
	out->bgimage = getAssetByName(out->vector, "World Background");
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
	free(world);
}

void zoom(SDL_World * world, double z){
	int cursorX, cursorY;

	
	SDL_GetMouseState(&cursorX,&cursorY);

	if(z * world->zoomFactor < 0.1  || z * world->zoomFactor >= 1)
		return;

	world->zoomFactor *= z;
	int qty = getQtyActiveAssets(world->vector);
	int i;
	for(i=0;i<qty;i++){
		if(world->vector->assets[i].original != NULL)
			modifyAssetImage(world->vector, i, 0, world->zoomFactor);
	}
	translateCamera(world,cursorX*(1.0-z),cursorY*(1.0-z));
}


void renderBorder(SDL_World * world, SDL_Surface * screen, int gridSize){

	int i;

	Uint32 gridColor = SDL_MapRGB( screen->format, 255, 255, 255 );

	// Verticales
	for(i=0;i<world->sizeX+1;i+=world->sizeX)
		SDL_printLine(screen, world->cameraX + i*gridSize * world->zoomFactor, world->cameraY, world->cameraX + i*gridSize * world->zoomFactor, world->sizeY*world->gridSize*world->zoomFactor + world->cameraY, gridColor );

	// Horizontales
	for(i=0;i<world->sizeY+1;i+=world->sizeY)
		SDL_printLine(screen, world->cameraX , world->cameraY + i*gridSize * world->zoomFactor, world->sizeX*world->gridSize*world->zoomFactor + world->cameraX, world->cameraY + i*gridSize * world->zoomFactor, gridColor );
}

void renderGrid(SDL_World * world, SDL_Surface * screen, int gridSize){

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

	int i,j,k;

	int qtyBgsX = 2 + screen->w / world->bgimage->image->w;
	int qtyBgsY = 2 + screen->h / world->bgimage->image->h;

	int dX = world->cameraX % world->bgimage->image->w;
	int dY = world->cameraY % world->bgimage->image->h;

	//printf("Voy a dibujar fondo\n");
	for(j=-1;j<qtyBgsY;j++)
		for(i=-1;i<qtyBgsX;i++)
			blitSurface( screen, world->bgimage->image, i*world->bgimage->image->w + dX, j*world->bgimage->image->h + dY);


	//renderGrid(world,screen, world->gridSize);

//	printf("Hola\n");
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
	//printf("Chau\n");

	// Overlay border
	renderBorder(world,screen, world->gridSize);

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

	int frames;
	// Only works for animated assets (or assets that have frames)
	if(world->grid[layer][x][y].animated){
		if(world->grid[layer][x][y].oriented)	// If oriented, h is 4 times larger, for orientation sprites 
			frames = world->vector->assets[index].image->w / (world->vector->assets[index].image->h/4);
		else					
			frames = world->vector->assets[index].image->w / world->vector->assets[index].image->h;
	}

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
// If swap is true, then swap objects in same layer (using tmp layer SWAP_LAYER)
int moveObject(SDL_World * world, int fromX, int fromY, int toX, int toY, int layer, int swap){

	if(swap -1 > SWAP_OFFSET ){
		printf("Bad swap offset\n");
		exit(1);
	}


	if( verifyGrid(world, fromX, fromY, layer) && verifyGrid(world, toX, toY, layer)){
	
		int index = world->grid[layer][fromX][fromY].id;

		// If object doesn't exist, finish
		if(index <0)
			return 1;

		// If asset exists and position is valid
		if( fromX != toX || fromY != toY ){

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
					(*offset) += (*dG) / (double)frames;	// frames cant be 0, this would imply image->w is 0
					oldGrid->frame++;
				}

				// When finally moved,
				if(fabs((*offset) - (*dG)) <= 0.1 ){

					// If it has to swap objects in same layer
					if(swap){
		
						// swap > 0 means layer offset
						int swapLayer = SWAP_LAYER + swap - 1;
	
						// Check if other cell has already finished, and swap
						if(nextGrid->id == -1 ){

							// Fill other cell data with my data
							nextGrid->id = index;
							nextGrid->offsetX = 0;
							nextGrid->offsetY = 0;
							nextGrid->frame = 1;
							nextGrid->animated = oldGrid->animated;
							nextGrid->oriented = oldGrid->oriented;
							nextGrid->orientation = oldGrid->orientation;

							// Fill my cell with other cell data (which is in SWAP_LAYER
							GridObject * swapGrid =  &world->grid[swapLayer][toX][toY];

							oldGrid->id = swapGrid->id;
							oldGrid->offsetX = 0;
							oldGrid->offsetY = 0;
							oldGrid->frame = 1;
							oldGrid->animated = swapGrid->animated;
							oldGrid->oriented = swapGrid->oriented;
							oldGrid->orientation = swapGrid->orientation;
					
							// And erase swap cell data
							swapGrid->id = -1;
							swapGrid->offsetX = 0;
							swapGrid->offsetY = 0;
							swapGrid->frame = 1;
							swapGrid->animated = !ANIMATED;
							swapGrid->oriented = !ORIENTED;
							swapGrid->orientation = SPRITE_DOWN; // Down is default

							// Too verbose, i know.. :S
						} else {
							// Else, i must wait other cell to swap myself, so
							// erase my cell and move it to swap layer

							GridObject * swapGrid =  &world->grid[swapLayer][fromX][fromY];
							
							swapGrid->id = oldGrid->id;
							swapGrid->offsetX = 0;
							swapGrid->offsetY = 0;
							swapGrid->frame = 1;
							swapGrid->animated = oldGrid->animated;
							swapGrid->oriented = oldGrid->oriented;
							swapGrid->orientation = oldGrid->orientation;

							// And erase swap cell data
							oldGrid->id = -1;
							oldGrid->offsetX = 0;
							oldGrid->offsetY = 0;
							oldGrid->frame = 1;
							oldGrid->animated = !ANIMATED;
							oldGrid->oriented = !ORIENTED;
							oldGrid->orientation = SPRITE_DOWN; // Down is default

						}

					} else {
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

					}
					return 1; // finished moving
				}
			}
		}
	} else{
	 printf("Falle: from (%d,%d) to (%d,%d), layer:%d, sizeX %d sizeY %d \n", fromX, fromY, toX,toY, layer, world->sizeX, world->sizeY);
		exit(1);
	}

	return 0;	// still moving
}
