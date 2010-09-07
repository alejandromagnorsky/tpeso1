#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "../include/common.h"
#include "../include/communication.h"
#include "../include/map.h"

typedef enum { north, east, south, west, northwest, northeast, southeast, southwest } Cardinal;

// If the variable Pos is called mov it refers to relative positions (up, down, left or right)
// Else, the variable Pos is an absolut position

typedef struct{
	OpCode opCode;
	FoodType food;
	Pos auxPos;
	Pos anthill;
	int key;
}Ant;

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

Pos getCurrentPos(int key);
