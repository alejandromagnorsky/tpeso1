#include <stdio.h>
#include <stdlib.h>
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

int action(Ant * ant);

void goAnthill(Ant * ant);

void search(Ant * ant);

bool getNearFood(Ant * ant);

bool randomMove(Ant * ant, bool trace);

bool move(Pos to, bool trace, int key);

Cardinal getCardinal(Pos from, Pos to);

void setRegister(Ant * ant);

bool setFood(Pos to, int key);

Pos getCurrentPos(int key);
