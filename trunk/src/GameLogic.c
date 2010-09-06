#include "../include/GameLogic.h"

Command * commands = NULL;
int commandsSize;
int EOT = 1;
int turn = 0;

void startGame(SDL_Surface * screen){

	SDL_World * gameWorld = initGame(screen);

	gameLoop(gameWorld, screen);

	// exit Game ( destroy thread )
}

SDL_World * initGame(SDL_Surface * screen){
	SDL_World * out = getSDLWorld(15, 15, "assets/bg.jpg", "JPG", SDL_MapRGB( screen->format, 0, 0, 0 ) );

	addAsset(out->vector, "assets/planta.png", "PNG", "Planta", 1);
	addAsset(out->vector, "assets/tesoro.png", "PNG", "Tesoro", 1);
	addAsset(out->vector, "assets/fuente.png", "PNG", "Fuente", 1);
	addAsset(out->vector, "assets/test_flare.png", "PNG", "Flare", 1);

	int i;
	for(i=0;i<5;i++){
		addObject(out, "Planta", 2+i*2, 5, 0,!ANIMATED,!ORIENTED );	
		addObject(out, "Planta", 2+i*2, 10, 0,!ANIMATED,!ORIENTED );
	}	

	addObject(out, "Tesoro", 10, 7, 0,!ANIMATED,!ORIENTED );
	addObject(out, "Fuente", 10, 7, 0,!ANIMATED,!ORIENTED );

	return out;
}

void addRegisterCommand(Command c){

	// Wait till frontend executed commands
	while(turn);

	int i;
	for(i=0;i<commandsSize;i++)
		if(commands[i].valid == 0 ){
			commands[i].fromX = c.fromX;
			commands[i].fromY = c.fromY;
			
			commands[i].op = RegisterCommand;
			commands[i].valid = 1;
			return;	// Command added
		}
	return;
}

// Add commands to the queue
void addMoveCommand(Command c){
	int i;

	// Wait till frontend executed commands
	while(turn);

	for(i=0;i<commandsSize;i++)
		if(commands[i].valid == 0 ){
			commands[i].fromX = c.fromX;
			commands[i].fromY = c.fromY;
			commands[i].toX = c.toX;
			commands[i].toY = c.toY;
			
			commands[i].op = MoveCommand;
			commands[i].valid = 1;
			return;	// Command added
		}

}

// Basically execute commands.
// Return 0 when commands yet not executed
// Return 1 when all commands executed
int executeMoveCommands(SDL_World * gameWorld){
	int i;
	int finished = 1;
	int finishedTotal = 1;
	for(i=0;i<commandsSize;i++)
		if(commands[i].valid == 1 && commands[i].op == MoveCommand){
				finished = moveObject(gameWorld, commands[i].fromX, commands[i].fromY,commands[i].toX, commands[i].toY, 1); // Ants work in layer 1
				if(finished)
					commands[i].valid = 0;
				finishedTotal = finishedTotal && finished;
		}
	return finishedTotal;
}



// Basically execute commands.
// Return 0 when commands yet not executed
// Return 1 when all commands executed
void executeRegisterCommands(SDL_World * gameWorld){
	int i;
	for(i=0;i<commandsSize;i++)
		if(commands[i].valid == 1 && commands[i].op == RegisterCommand ){
			addObject(gameWorld, "Flare",commands[i].fromX, commands[i].fromY, 1, ANIMATED,!ORIENTED);
			commands[i].valid = 0;
		}

}

int checkEOT(){
	if(EOT){
		EOT = 0;
		return 1;
	}
	return 0;
}

void gameLoop(SDL_World * gameWorld, SDL_Surface * screen){

	int i;
	int antQty = 20;

	// Initialize commands 	
	commandsSize = antQty;
	commands = calloc(antQty, sizeof(Command));
	for(i=0;i<antQty;i++)
		commands[i].valid = 0;
		
	turn = 0;

	while(1){

		// Pan & zoom
		if(getUserInput(gameWorld) == -1)
			return;

		// Check if turn has ended
		if(!turn){
			turn = checkEOT();
		// If turn has ended, wait till all commands are executed 
		} else if(executeMoveCommands(gameWorld)){
	//			printf("EOT!\n");
				executeRegisterCommands(gameWorld);
				turn = 0;
			}
		// Render world
		renderSDLWorld(gameWorld, screen);

		/* Update screen */
		SDL_Flip(screen);  

		/* Delay */
		SDL_Delay( DELAY );
	
	}
}

int getUserInput(SDL_World * gameWorld){

	SDL_Event event;
	Uint8 * keystate;

	while( SDL_PollEvent( &event ) ){
            /* See if user  quits */
		if(event.type == SDL_QUIT )
			return -1;
		else if(event.type == SDL_MOUSEBUTTONDOWN)
			switch(event.button.button){
				case SDL_BUTTON_WHEELUP:
					zoom(gameWorld,1.1);
					break;
				case SDL_BUTTON_WHEELDOWN:
					zoom(gameWorld,0.9);
					break;
			}
	}

	/* Get a keyboard snapshot */
	keystate = SDL_GetKeyState( NULL );

	// If user presses ESC
	if(keystate[SDLK_ESCAPE])
		return -1;

	if(keystate[SDLK_DOWN])	translateCamera(gameWorld,0,-SENSITIVITY);
	if(keystate[SDLK_UP])	translateCamera(gameWorld,0,SENSITIVITY);
	if(keystate[SDLK_LEFT])	translateCamera(gameWorld,SENSITIVITY,0);
	if(keystate[SDLK_RIGHT])translateCamera(gameWorld,-SENSITIVITY,0);

	return 0;
}


