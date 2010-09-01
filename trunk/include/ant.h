#include <stdio.h>
#include <stdlib.h>
#include "../include/common.h"
#include "../include/communication.h"
#include "../include/map.h"

typedef enum { north, east, south, west, northwest, northeast, southeast, southwest } Cardinal;

typedef struct{
	FoodType food;
	Pos currentPos;
	Pos anthill;
}Ant;

int vecMov[4][2] = {{0,1}, {1,0}, {0,-1}, {-1,0}}; // Represents: up, right, down and left


int action(Ant * ant);

void goAnthill(Ant * ant);

bool getNearFood(Ant * ant);

bool randomMove(Ant * ant);

bool move(Pos to, bool trace);

Cardinal getCardinal(Pos from, Pos to);

bool followTrace();

void setRegister(Ant * ant);

void setFood(Ant * ant);

Pos getCurrentPos(void);
