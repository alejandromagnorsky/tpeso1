#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "../include/common.h"
#include "../include/communication.h"


typedef enum { north, east, south, west, northwest, northeast, southeast, southwest } Cardinal;
typedef enum {GET_FOOD, FOLLOW_TRACE, SET_SHOUT, FOLLOW_SHOUT, SMELL} AntOp;

// If the variable Pos is called mov it refers to relative positions (up, down, left or right)
// Else, the variable Pos is an absolute position

typedef struct{
	AntOp op;
	bool food;
	Pos auxPos;
	Pos anthill;
	int key;
}Ant;

typedef struct{
	int intensity; // If 1 the scream is no valid yet. If 0 the scream is valid. If -1 there is no scream
	Pos pos;
} Scream;

extern Scream * screams;

void * antMain(void * arg);

bool action(Ant * ant);

bool goAnthill(Ant * ant);

void search(Ant * ant);

bool randomMove(Ant * ant, bool trace);

bool move(Pos to, bool trace, int key);

Cardinal getCardinal(Pos from, Pos to);

void setRegister(Ant * ant);

bool getNearFood(Ant * ant, Pos to);

bool setFood(Ant * ant, Pos to);

void setShout(Ant * ant);

bool followShout(Ant * ant);

bool getNearestScream(Ant * ant);

bool hasShouted(Ant * ant, Pos to);

void reduceScreamIntensity(Ant * ant);

int getDistance(Pos from, Pos to);

Pos getCurrentPos(int key);
