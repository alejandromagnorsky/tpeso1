# Makefile
# Ant Colony Simulation
#	 Magnorsky, Alejandro
#	 Mata Suárez, Andrés
#	 Merchante, Mariano
#

# Flags
CC =		gcc
CFLAGS =	-g -Wall -lm -lrt -o
SDLFLAGS =	-Iinclude/SDL `sdl-config --libs --cflags` -Llib -lSDL -lSDL_image -lSDL_mixer

# Sources
FRONTEND =	src/frontend/GameLogic.c \
		src/frontend/SDLMainWindow.c \
		src/frontend/SDL_rotozoom.c \
		src/frontend/SDL_utils.c \
		src/frontend/SDL_World.c \
		src/frontend/SDL_AssetManager.c

BACKEND =	src/backend/communication.c \
		src/backend/IPC/comm-$(IPC).c

map :   $(FRONTEND) $(BACKEND) src/backend/map.c src/backend/anthill.c src/backend/ant.c
	$(CC) $(CFLAGS) map	$(BACKEND) $(FRONTEND)	src/backend/map.c $(SDLFLAGS)
	$(CC) $(CFLAGS) anthill $(BACKEND)		src/backend/anthill.c src/backend/ant.c


