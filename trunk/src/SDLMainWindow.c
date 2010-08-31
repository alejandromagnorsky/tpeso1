#include <SDL.h>
#include <signal.h>
#include <unistd.h>
#include <SDL_rotozoom.h>
#include "../include/SDL_utils.h"
#include "../include/SDL_World.h"
#include "../include/SDL_AssetManager.h"
#include "../include/communication.h"
#include <pthread.h>

#define SCREEN_WIDTH 644
#define SCREEN_HEIGHT 480

#define SENSITIVITY 3

#define DELAY 30	// delay in ms

Message * msg = NULL;
bool newMessage = false;

int cleanUp( int err )
{
	destroyIPC();
	SDL_Quit();
	return err;            
}

void * checkMessages(void * threadid){

	openIPC();

	while(1){
		if(!newMessage){
			msg = receiveMessage(SERVER);
			newMessage = true;
		}
	}

	closeIPC();

	pthread_exit(NULL);
}

void startMapEngine(){

	int frontendPID = getpid();

	int pid = fork();
	char * argPID = malloc(10*sizeof(char));

	sprintf(argPID, "%d", frontendPID);

	switch(pid){
		case -1: 
			printf("can't fork\n");
			exit(-1);	
			break;
		case 0 : // this is the code the child runs 
			execl ("./map","map",argPID, NULL);
			break;
		default: // this is the code the parent runs 
			// Get back to frontend
			break;
	}
}


void printAnts( SDL_Surface *screen, SDLWorld * world, SDL_Surface * antImage, int ants[30][30]){
	int i,j;
	for(i=0;i<30;i++)
		for(j=0;j<30;j++)
			if(ants[i][j])			
				renderObject(screen, world, antImage, j, i);
}

void getInfoFromBackend(int ants[30][30]){
	if(msg->opCode == MOVE){
		if(msg->param == SET)
			ants[msg->pos.x][msg->pos.y] = 1;
		else if(msg->param == EMPTY)
			ants[msg->pos.x][msg->pos.y] = 0;
	}

	newMessage = false;
}



int main(int argc, char * argv[]){

	startMapEngine();

	SDL_Surface * screen;
	SDL_Event event;
	Uint8 * keystate;

	/* Initialize video, or at least try */
	if( (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) != 0 )
	{
		fprintf(stderr, "SDL could not be initialized: %s\n", SDL_GetError());
		return 1;
	}

	/* Initialize video mode */
	if( (screen = SDL_SetVideoMode(SCREEN_WIDTH,
	       SCREEN_HEIGHT, 32, SDL_SWSURFACE | SDL_ANYFORMAT | SDL_HWPALETTE )) == NULL )
	{
		fprintf(stderr, "SDL could not set video mode: %s\n", SDL_GetError());
		return cleanUp( 1 );
	}


	signal(SIGINT, sigHandler);

	SDL_WM_SetCaption("Simple Ant Colony simulation", NULL);

	SDLWorld * world = getWorld(696, 1050, "assets/bg.jpg", "JPG", SDL_MapRGB( screen->format, 0, 0, 0 ) );

	addAsset(world->vector, "assets/hormiga_tmp.jpg", "JPG", "Hormiga", 1);

	SDL_Surface * hormiga = getAssetImage(world->vector, "Hormiga");

	pthread_t thread;
	pthread_create(&thread, NULL, checkMessages, NULL);

	int ants[30][30];

	int i,j;
	for(i=0;i<30;i++)
		for(j=0;j<30;j++)
			ants[i][j] = 0;


  	while(1)
     	{
		while( SDL_PollEvent( &event ) ){
                    /* See if user  quits */
			if(event.type == SDL_QUIT )
				return cleanUp(1);
			else if(event.type == SDL_MOUSEBUTTONDOWN)
				switch(event.button.button){
					case SDL_BUTTON_WHEELUP:
						zoom(world,1.1);
						break;
					case SDL_BUTTON_WHEELDOWN:
						zoom(world,0.9);
						break;
				}
		
		}

		/* Get a keyboard snapshot */
		keystate = SDL_GetKeyState( NULL );

		// If user presses ESC
		if(keystate[SDLK_ESCAPE])
			return cleanUp(1);;

		if(keystate[SDLK_DOWN])	translateCamera(world,0,-SENSITIVITY);
		if(keystate[SDLK_UP])	translateCamera(world,0,SENSITIVITY);
		if(keystate[SDLK_LEFT])	translateCamera(world,SENSITIVITY,0);
		if(keystate[SDLK_RIGHT])translateCamera(world,-SENSITIVITY,0);
		if(keystate[SDLK_a]) zoom(world,1.1);
		if(keystate[SDLK_z]) zoom(world,0.9);

		renderSDLWorld(world, screen);
		renderObject(screen, world, hormiga, 2, 3);
		if(newMessage)
			getInfoFromBackend(ants);
		printAnts( screen, world, hormiga, ants);

		/* Update screen */
		SDL_Flip(screen);  

		/* Delay */
//		SDL_Delay( DELAY );
     }   

	endWorld(world);
    SDL_FreeSurface(screen);
    return cleanUp(0);
}
