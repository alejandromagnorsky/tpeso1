#include "../include/GameLogic.h"

Message * frontendMessage = NULL;

void startGame(SDL_Surface * screen){

	openIPC();

	SDL_World * gameWorld = initGame(screen);

	gameLoop(gameWorld, screen);

	// exit Game ( destroy thread )

	closeIPC();
}

// Returns 1 if not finished registering
// Returns 0 if finished registering
int registerAnts(SDL_World * gameWorld){

	if(frontendMessage != NULL &&
	   frontendMessage->opCode == REGISTER)
	{
		Pos pos = frontendMessage->pos;

		if(frontendMessage->param == SET){
			addObject(gameWorld, "Flare", pos.x, pos.y, 1, ANIMATED,!ORIENTED);
			frontendMessage = NULL;
			return 1;
		}
		if(frontendMessage->param == OK){
			frontendMessage = NULL;
			return 0;
		}
	}	
	return 1;
}

SDL_World * initGame(SDL_Surface * screen){
	SDL_World * out = getWorld(15, 15, "assets/bg.jpg", "JPG", SDL_MapRGB( screen->format, 0, 0, 0 ) );

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


/* Just get messages from server */
void * getBackendInput(void * threadid){


	while(1){
		if(frontendMessage == NULL){
			frontendMessage = receiveMessage(SERVER);

			// If message is invalid semantically...
			if(frontendMessage->opCode != MOVE && 
			frontendMessage->opCode != TURN &&
			frontendMessage->opCode != REGISTER)
				frontendMessage = NULL;
		}
	}

	pthread_exit(NULL);

	return NULL;
}

// Add commands to the queue
void addMoveCommand(MoveCommand * commands, int size){
	int i;

	if(frontendMessage != NULL &&
	   frontendMessage->opCode == MOVE &&
	   frontendMessage->param == SET)
	{
	
		Pos from = frontendMessage->fromPos;

		// Remember, Message.pos is normalized
		Pos to = frontendMessage->fromPos;
		to.x += frontendMessage->pos.x;
		to.y += frontendMessage->pos.y;
	
		for(i=0;i<size;i++)
			if(commands[i].valid == 0 ){
				commands[i].fromX = from.x;
				commands[i].fromY = from.y;
				commands[i].toX = to.x;
				commands[i].toY = to.y;

				
				commands[i].valid = 1;
				frontendMessage = NULL;	// Message received, done
				return;	// Command added
			}
	}	

}

// Basically execute commands.
// Return 0 when commands yet not executed
// Return 1 when all commands executed
int executeMoveCommands(SDL_World * gameWorld, MoveCommand * commands, int size){

	int i;
	int finished = 1;
	int finishedTotal = 1;
	for(i=0;i<size;i++)
		if(commands[i].valid == 1 ){
			finished = moveObject(gameWorld, commands[i].fromX, commands[i].fromY,commands[i].toX, commands[i].toY, 1); // Ants work in layer 1
			if(finished)
				commands[i].valid = 0;
			finishedTotal = finishedTotal && finished;
		}

	return finishedTotal;
}

int checkEOT(){
	if(frontendMessage != NULL)
		if(frontendMessage->opCode == TURN &&
		frontendMessage->param == SET){
			frontendMessage = NULL;
			return 1;
		}
	return 0;
}

void gameLoop(SDL_World * gameWorld, SDL_Surface * screen){

	// Constantly get input from backend, parallel to rendering
	pthread_t backendInputThread;
	pthread_create(&backendInputThread, NULL, getBackendInput, NULL);

	// Render temporary world while loading.
	while(registerAnts(gameWorld)){
		// Pan & zoom
		if(getUserInput(gameWorld) == -1)
			return;
		
		renderSDLWorld(gameWorld, screen);
		/* Update screen */
		SDL_Flip(screen);  
	}


	int i;
	int antQty = 10;
	MoveCommand * commands = calloc(antQty, sizeof(MoveCommand));

	for(i=0;i<antQty;i++)
		commands[i].valid = 0;
		

	int turn = 0;

	while(1){

		// Pan & zoom
		if(getUserInput(gameWorld) == -1)
			return;

		// If turn hasnt ended, get commands, check EOT
		if(!turn){
			// Parse messages, add move commands
			addMoveCommand(commands, antQty);						
			turn = checkEOT();
		// If turn has ended, wait till all commands are executed 
		} else {		
			if(executeMoveCommands(gameWorld,commands, antQty))
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

	if(keystate[SDLK_d]){
		if(moveObject(gameWorld, 5, 3, 6, 3, 0) )
			printf("lalala\n");
		 else printf("trololo\n");
	}

	if(keystate[SDLK_a]){
		if(moveObject(gameWorld, 6, 3, 5, 3, 0) )
			printf("lalala\n");
		 else printf("trololo\n");
	}

	return 0;
}


