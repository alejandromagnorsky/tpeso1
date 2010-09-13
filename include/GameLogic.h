#include <SDL.h>
#include <SDL_mixer.h>
#include <time.h>
#include <SDL_rotozoom.h>
#include "../include/SDL_utils.h"
#include "../include/SDL_World.h"
#include "../include/SDL_AssetManager.h"
#include "../include/transport.h"
#include <pthread.h>

#define AVL_SOUND_CHANNEL -1
#define CHANNEL AVL_SOUND_CHANNEL
#define PLAYS 1
#define LOOP PLAYS-1

#define SCREEN_WIDTH 644
#define SCREEN_HEIGHT 480

#define SENSITIVITY 15
#define ZOOM 0.1
#define DELAY 10	// delay in ms

#define ANT_LAYER 4
#define FOOD_LAYER 5
#define TRACE_LAYER 3
#define BG_LAYER 0

#define MAX_SOUNDS 3

typedef enum { MoveFoodCommand, MoveAntCommand, RegisterAnthillCommand, \
	RegisterCommand, RegisterFoodCommand, RegisterBigFoodCommand, DeleteFoodCommand, SetTraceCommand, DeleteTraceCommand } CommandOp;

typedef struct{
	int fromX,fromY;
	int toX,toY;	
	int valid;
	CommandOp op;

	union {
		int points;
		double trace;
	} extra;
} Command; 


extern int EOT; // End of turn
extern pthread_mutex_t EOT_mutex;
extern pthread_cond_t EOT_cond;

extern Mix_Chunk * sounds[MAX_SOUNDS];

void startGame(SDL_Surface * screen, int sizeX, int sizeY);

void registerAnts(SDL_World * gameWorld);

SDL_World * initGame(SDL_Surface * screen, int sizeX, int sizeY);

void gameLoop(SDL_World * world, SDL_Surface * screen);

int getUserInput(SDL_World * gameWorld);

int executeMoveCommands(SDL_World * gameWorld);

void addCommand(Command c);

// SDL SOUNDS
void shout();
void playMusic();
void openSounds();
void playSound(Mix_Chunk * sound);
void closeSounds();

int checkEOT();

