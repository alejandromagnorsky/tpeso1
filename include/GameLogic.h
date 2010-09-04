#include <SDL.h>
#include <SDL_rotozoom.h>
#include "../include/SDL_utils.h"
#include "../include/SDL_World.h"
#include "../include/SDL_AssetManager.h"
#include "../include/communication.h"
#include <pthread.h>

#define SCREEN_WIDTH 644
#define SCREEN_HEIGHT 480

#define SENSITIVITY 25

#define DELAY 15	// delay in ms

typedef enum { MoveCommand, RegisterCommand } CommandOp;

typedef struct{
	int fromX,fromY;
	int toX,toY;	
	int valid;
	CommandOp op;
} Command;

void startGame(SDL_Surface * screen, int key);

void registerAnts(SDL_World * gameWorld);

SDL_World * initGame(SDL_Surface * screen);

void gameLoop(SDL_World * world, SDL_Surface * screen);

int getUserInput(SDL_World * gameWorld);

void * getBackendInput(void * threadid);

int executeMoveCommands(SDL_World * gameWorld, Command * commands, int size);

void addRegisterCommand(Command * command, int size);


int checkEOT();

