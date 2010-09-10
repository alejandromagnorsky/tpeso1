#include "../include/GameLogic.h"

#define COMMAND_SIZE_THRESHOLD 50

/*
	Disclaimer: Es un desastre la logica del frontend, como no era el principal
	problema, se le presto menos atencion (y menos tiempo de desarrollo/diseño)
	Asi que sepan entender :P
*/

pthread_mutex_t EOT_mutex =  PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t EOT_cond = PTHREAD_COND_INITIALIZER;



pthread_mutex_t commands_mutex =  PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t commands_cond = PTHREAD_COND_INITIALIZER;

Command * commands = NULL;
int commandsSize;

int EOT = 0;

void startGame(SDL_Surface * screen, int sizeX, int sizeY){

	SDL_World * gameWorld = initGame(screen,sizeX, sizeY);

	gameLoop(gameWorld, screen);

	// exit Game ( destroy thread )
}

SDL_World * initGame(SDL_Surface * screen, int sizeX, int sizeY){
	SDL_World * out = getSDLWorld(sizeX, sizeY, "assets/bg.jpg", "JPG", SDL_MapRGB( screen->format, 0, 0, 0 ) );

	addAsset(out->vector, "assets/anthill.png", "PNG", "Anthill", ALPHA);
	addAsset(out->vector, "assets/ant.png", "PNG", "Ant", ALPHA);
	addAsset(out->vector, "assets/trace.png", "PNG", "Trace", ALPHA);
	addAsset(out->vector, "assets/smallFood1.png", "PNG", "SmallFood1", ALPHA);
	addAsset(out->vector, "assets/smallFood2.png", "PNG", "SmallFood2", ALPHA);
	addAsset(out->vector, "assets/bigFood2.png", "PNG", "BigFood2", ALPHA);
	addAsset(out->vector, "assets/bigFood1.png", "PNG", "BigFood1", ALPHA);

	return out;
}

// Add commands to the queue
// Note that this function is ALWAYS CALLED BY THE MAP THREAD
// This may cause synchronization problems, so mutex and condvars are used
// because map thread may try to send commands before commands vector initialized properly
void addCommand(Command c){
	int i;

	pthread_mutex_lock(&commands_mutex);

	if(commands == NULL)
		pthread_cond_wait(&commands_cond, &commands_mutex);

	for(i=0;i<commandsSize;i++)
		if(commands[i].valid == 0 ){
			commands[i].fromX = c.fromX;
			commands[i].fromY = c.fromY;
			commands[i].toX = c.toX;
			commands[i].toY = c.toY;
			commands[i].op = c.op;
			commands[i].extra = c.extra;
			commands[i].valid = 1;
			pthread_mutex_unlock(&commands_mutex);
			return;	// Command added
		}

	pthread_mutex_unlock(&commands_mutex);
}

// Basically execute commands.
// Return 0 when commands yet not executed
// Return 1 when all commands executed
int executeMoveCommands(SDL_World * gameWorld){
	int i;
	int finished = 1;
	int finishedTotal = 1;
	int layer = ANT_LAYER;
	for(i=0;i<commandsSize;i++){
		if(commands[i].valid == 1 && ( commands[i].op == MoveAntCommand || commands[i].op == MoveFoodCommand )  ){
				layer = commands[i].op == MoveAntCommand ? ANT_LAYER : FOOD_LAYER;
				finished = moveObject(gameWorld, commands[i].fromX, commands[i].fromY,commands[i].toX, commands[i].toY, layer);
				if(finished)
					commands[i].valid = 0;
				finishedTotal = finishedTotal && finished;
		}
	}
	return finishedTotal;
}

void executeDeleteFood(SDL_World * gameWorld,Command * comm){
	deleteObject(gameWorld, comm->fromX, comm->fromY,FOOD_LAYER );
	comm->valid = 0;
}

// Basically execute Food commands.
void executeRegisterFood(SDL_World * gameWorld,Command * comm){
	char * name = malloc(sizeof(char)*10);
	sprintf(name, "SmallFood%d", rand()%2 + 1 );

	addObject(gameWorld, name,comm->fromX, comm->fromY, FOOD_LAYER, !ANIMATED,!ORIENTED);
	comm->valid = 0;
}


// Basically execute Food commands.
void executeRegisterBigFood(SDL_World * gameWorld, Command * comm){
	char * name = malloc(sizeof(char)*10);
	sprintf(name, "BigFood%d", rand()%2 + 1 );

	addObject(gameWorld, name,comm->fromX, comm->fromY, FOOD_LAYER, !ANIMATED,!ORIENTED);
	comm->valid = 0;
}

// I HATE SDL DOESNT HAVE PER PIXEL BLENDING AND SURFACE BLENDING SIMULTANEOUSLY!!!
void executeSetTrace(SDL_World * gameWorld, Command * comm){
	// Clear trace before, so it can be updated
	deleteObject(gameWorld,comm->fromX, comm->fromY, TRACE_LAYER );

	int alpha = (1.0 - comm->extra.trace) * 10 + 1;
	addObject(gameWorld, "Trace",comm->fromX, comm->fromY, TRACE_LAYER, ANIMATED,!ORIENTED);
	setFrame(gameWorld, comm->fromX, comm->fromY, TRACE_LAYER, alpha); 
	comm->valid = 0;
}

void executeDeleteTrace(SDL_World * gameWorld, Command * comm){
	deleteObject(gameWorld,comm->fromX, comm->fromY, TRACE_LAYER );
	comm->valid = 0;
}


void executeRegisterAnthill(SDL_World * gameWorld, Command * comm){
	addObject(gameWorld, "Anthill",comm->fromX, comm->fromY, BG_LAYER, !ANIMATED,!ORIENTED);
	comm->valid = 0;
}

void executeRegisterCommands(SDL_World * gameWorld, Command * comm){
	addObject(gameWorld, "Ant",comm->fromX, comm->fromY, ANT_LAYER, ANIMATED,ORIENTED);
	comm->valid = 0;
}


void gameLoop(SDL_World * gameWorld, SDL_Surface * screen){

	int i;
	int antQty = 20;

	pthread_mutex_lock(&commands_mutex);

	// Initialize commands, should use mutex
	commandsSize = antQty + COMMAND_SIZE_THRESHOLD;
	commands = calloc(commandsSize, sizeof(Command));
	for(i=0;i<antQty;i++)
		commands[i].valid = 0;

	pthread_cond_signal(&commands_cond);

	pthread_mutex_unlock(&commands_mutex);

	while(1){

		// Pan & zoom
		if(getUserInput(gameWorld) == -1)
			return;
	
		pthread_mutex_lock(&EOT_mutex);

		// If turn has ended, wait till all commands are executed 
		if(EOT)
			if(executeMoveCommands(gameWorld)){

				for(i=0;i<commandsSize;i++)
					if(commands[i].valid == 1 )
					switch(commands[i].op ){
						case RegisterCommand: executeRegisterCommands(gameWorld, commands+i); break;
						case RegisterBigFoodCommand: executeRegisterBigFood(gameWorld,commands+i); break;
						case RegisterFoodCommand: executeRegisterFood(gameWorld, commands+i); break;
						case RegisterAnthillCommand: executeRegisterAnthill(gameWorld,commands+i); break;
						case DeleteFoodCommand:	executeDeleteFood(gameWorld, commands+i); break;
						case SetTraceCommand: executeSetTrace(gameWorld,commands+i); break;
						case DeleteTraceCommand: executeDeleteTrace(gameWorld,commands+i); break;
						default:break;
					}

				EOT = 0;
				// Signal map that frontend finished executing.
				pthread_cond_signal(&EOT_cond);
			}

		pthread_mutex_unlock(&EOT_mutex);

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

void shout(){
	struct Mix_Chunk * shout;
	shout = openSound("test.wav");
	playSound(shout);
	//closeSounds(shout);
}

Mix_Chunk * openSound(char * file){
	struct Mix_Chunk * sound;
	if((sound = Mix_LoadWAV("test.wav")) == NULL) {
		fprintf(stderr, "Unable to load WAV file: %s\n", Mix_GetError());
		exit(1);
	}
	return sound;
}
void playSound(Mix_Chunk * sound){
	int channel;
	channel = Mix_PlayChannel(CHANNEL, sound, LOOP);
	if(channel == -1) {
		fprintf(stderr, "Unable to play WAV file: %s\n", Mix_GetError());
		exit(1);
	}

	while(Mix_Playing(channel) != 0);
}

void closeSounds(/*Mix_Chunk * sound*/){
	//Mix_FreeChunk(sound);
	Mix_CloseAudio();
}

