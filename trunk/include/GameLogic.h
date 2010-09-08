#include <SDL.h>
#include <SDL_rotozoom.h>
#include "../include/SDL_utils.h"
#include "../include/SDL_World.h"
#include "../include/SDL_AssetManager.h"
#include "../include/communication.h"
#include <pthread.h>

#define SCREEN_WIDTH 644
#define SCREEN_HEIGHT 480

#define SENSITIVITY 30

#define DELAY 5	// delay in ms

typedef enum { MoveCommand, RegisterCommand } CommandOp;

typedef struct{
	int fromX,fromY;
	int toX,toY;	
	int valid;
	CommandOp op;
} Command;

extern int EOT; // End of turn
extern pthread_mutex_t EOT_mutex;
extern pthread_cond_t EOT_cond;



void startGame(SDL_Surface * screen);

void registerAnts(SDL_World * gameWorld);

SDL_World * initGame(SDL_Surface * screen);

void gameLoop(SDL_World * world, SDL_Surface * screen);

int getUserInput(SDL_World * gameWorld);

int executeMoveCommands(SDL_World * gameWorld);


void executeRegisterCommands(SDL_World * gameWorld);

void addRegisterCommand(Command c);

void addMoveCommand(Command c);

int checkEOT();

