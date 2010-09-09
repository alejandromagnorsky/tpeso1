#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_mixer.h>
#include <SDL_rotozoom.h>
#include "../include/SDL_utils.h"
#include "../include/SDL_World.h"
#include "../include/SDL_AssetManager.h"
#include "../include/communication.h"
#include <pthread.h>

#define AVL_SOUND_CHANNEL -1
#define CHANNEL AVL_SOUND_CHANNEL
#define PLAYS 1
#define LOOP PLAYS-1

#define SCREEN_WIDTH 644
#define SCREEN_HEIGHT 480

#define SENSITIVITY 15
#define DELAY 10	// delay in ms

#define ANT_LAYER 1
#define FOOD_LAYER 2
#define BG_LAYER 0

typedef enum { MoveFoodCommand, MoveAntCommand, RegisterAnthillCommand, RegisterCommand, RegisterFoodCommand, RegisterBigFoodCommand, DeleteFoodCommand } CommandOp;

typedef struct{
	int fromX,fromY;
	int toX,toY;	
	int valid;
	CommandOp op;
} Command;

extern int EOT; // End of turn
extern pthread_mutex_t EOT_mutex;
extern pthread_cond_t EOT_cond;

void startGame(SDL_Surface * screen, int sizeX, int sizeY);

void registerAnts(SDL_World * gameWorld);

SDL_World * initGame(SDL_Surface * screen, int sizeX, int sizeY);

void gameLoop(SDL_World * world, SDL_Surface * screen);

int getUserInput(SDL_World * gameWorld);

int executeMoveCommands(SDL_World * gameWorld);


void executeRegisterCommands(SDL_World * gameWorld);

void addCommand(Command c);

// SDL SOUNDS
void shout();
Mix_Chunk * openSound(char * file);
void playSound(Mix_Chunk * sound);
void closeSounds(/*Mix_Chunk * sound*/);

int checkEOT();

