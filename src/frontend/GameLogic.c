#include "../../include/GameLogic.h"

#define COMMAND_SIZE_THRESHOLD 50
#define SHOUT_BASE "assets/audio/shout_"
#define PATH_LENGTH 19
#define SHOUT_EXTENSION ".wav"
#define EXT_LENGTH 4

/*
	Disclaimer: Es un desastre la logica del frontend, como no era el principal
	problema, se le presto menos atencion (y menos tiempo de desarrollo/diseño)
	Asi que sepan entender :P
*/


int EOT = 0;

pthread_mutex_t EOT_mutex =  PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t EOT_cond = PTHREAD_COND_INITIALIZER;

Command * commands = NULL;
int commandsSize;

pthread_mutex_t commands_mutex =  PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t commands_cond = PTHREAD_COND_INITIALIZER;

// extern definition of sounds[] and MAX_SOUNDS in GameLogic.h
Mix_Chunk * sounds[MAX_SOUNDS];

int mouseClicked = 0;

void startGame(SDL_Surface * screen, int sizeX, int sizeY){

	SDL_World * gameWorld = initGame(screen,sizeX, sizeY);

	gameLoop(gameWorld, screen);

	// exit Game ( destroy thread )
}

void initWorldRandomization(SDL_World * gameWorld){

	int i;
	int x,y;

	int piedras = gameWorld->sizeX /((rand()%4)+1);
	for(i=0;i < piedras ;i+= 8){
		x = rand()%gameWorld->sizeX;
		y = rand()%gameWorld->sizeY;
		addObject(gameWorld, "Piedra",x, y, BG_LAYER, !ANIMATED,!ORIENTED);
	}
	
	int grietas = gameWorld->sizeX /((rand()%4)+1);
	for(i=0;i < grietas ;i+= 8){
		x = rand()%gameWorld->sizeX;
		y = rand()%gameWorld->sizeY;
		addObject(gameWorld, "Grieta",x, y, BG_LAYER, !ANIMATED,!ORIENTED);
	}
}

SDL_World * initGame(SDL_Surface * screen, int sizeX, int sizeY){

	SDL_World * out = getSDLWorld(sizeX, sizeY, "bg.jpg", "JPG", SDL_MapRGB( screen->format, 0, 0, 0 ) );

	addAsset(out->vector, "ant.png", "PNG", "Ant", ALPHA);
	addAsset(out->vector, "anthill.png", "PNG", "Anthill", ALPHA);
	addAsset(out->vector, "trace.png", "PNG", "Trace", ALPHA);
	addAsset(out->vector, "smallFood1.png", "PNG", "SmallFood1", ALPHA);
	addAsset(out->vector, "smallFood2.png", "PNG", "SmallFood2", ALPHA);
	addAsset(out->vector, "bigFood2.png", "PNG", "BigFood2", ALPHA);
	addAsset(out->vector, "bigFood1.png", "PNG", "BigFood1", ALPHA);


	// Eye candyness X TREMEEEEEEee !!!111oneONE!!11!
	addAsset(out->vector, "piedra1.png", "PNG", "Piedra", ALPHA);
	addAsset(out->vector, "grieta.png", "PNG", "Grieta", ALPHA);

	initWorldRandomization(out);

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

	// If not enough space
	if(i==commandsSize){

		// Realloc new space
		commandsSize += 15;
		commands = realloc(commands, commandsSize*sizeof(Command));

		// Save command
		commands[i].fromX = c.fromX;
		commands[i].fromY = c.fromY;
		commands[i].toX = c.toX;
		commands[i].toY = c.toY;
		commands[i].op = c.op;
		commands[i].extra = c.extra;
		commands[i].valid = 1;
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
	int swap = 0;
	for(i=0;i<commandsSize;i++){
		if(commands[i].valid == 1 && ( commands[i].op == MoveAntCommand || commands[i].op == MoveFoodCommand )  ){
				layer = commands[i].op == MoveAntCommand ? ANT_LAYER : FOOD_LAYER;
				if( commands[i].extra.swap > 0 ){
					if( commands[i].op == MoveAntCommand ) swap = 1; // swap offset 1
					else swap = 2; // swap offset 2
				} else swap = 0;
				finished = moveObject(gameWorld, commands[i].fromX, commands[i].fromY,commands[i].toX, commands[i].toY, layer,swap);
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
	char * name = malloc(sizeof(char)*15);
	sprintf(name, "SmallFood%d", rand()%2 + 1 );

	addObject(gameWorld, name,comm->fromX, comm->fromY, FOOD_LAYER, !ANIMATED,!ORIENTED);
	comm->valid = 0;
}


// Basically execute Food commands.
void executeRegisterBigFood(SDL_World * gameWorld, Command * comm){
	char * name = malloc(sizeof(char)*15);
	sprintf(name, "BigFood%d", rand()%2 + 1 );

	addObject(gameWorld, name,comm->fromX, comm->fromY, FOOD_LAYER, !ANIMATED,!ORIENTED);
	comm->valid = 0;
}

// I HATE SDL DOESNT HAVE PER PIXEL BLENDING AND SURFACE BLENDING SIMULTANEOUSLY!!!
void executeSetTrace(SDL_World * gameWorld, Command * comm){
	// Clear trace before, so it can be updated
	deleteObject(gameWorld,comm->fromX, comm->fromY, TRACE_LAYER );

	int frame = (1.0 - comm->extra.trace) * 4 + 1;
	addObject(gameWorld, "Trace",comm->fromX, comm->fromY, TRACE_LAYER, ANIMATED,!ORIENTED);
	setFrame(gameWorld, comm->fromX, comm->fromY, TRACE_LAYER, frame); 
	comm->valid = 0;
}

void executeDeleteTrace(SDL_World * gameWorld, Command * comm){
	deleteObject(gameWorld,comm->fromX, comm->fromY, TRACE_LAYER );
	comm->valid = 0;
}


void executeRegisterAnthill(SDL_World * gameWorld, Command * comm){
	addObject(gameWorld, "Anthill",comm->fromX, comm->fromY, TRACE_LAYER, !ANIMATED,!ORIENTED);
	comm->valid = 0;
}

void executeRegisterCommands(SDL_World * gameWorld, Command * comm){
	addObject(gameWorld, "Ant",comm->fromX, comm->fromY, ANT_LAYER, ANIMATED,ORIENTED);
	comm->valid = 0;
}


void gameLoop(SDL_World * gameWorld, SDL_Surface * screen){

	int i;
	pthread_mutex_lock(&commands_mutex);

	// Initialize commands
	commandsSize = COMMAND_SIZE_THRESHOLD;
	commands = calloc(commandsSize, sizeof(Command));
	for(i=0;i<commandsSize;i++)
		commands[i].valid = 0;

	pthread_cond_signal(&commands_cond);

	pthread_mutex_unlock(&commands_mutex);


	SDL_Surface * cursor = loadImageSDL("assets/cursor_base.png", "PNG", ALPHA);
	SDL_Surface * moveCursor = loadImageSDL("assets/cursor_move.png", "PNG", ALPHA);

	SDL_ShowCursor(SDL_DISABLE);
	int cursorX,cursorY;


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
						case ExitCommand: return; break;
						default:break;
					}

				EOT = 0;
				// Signal map that frontend finished executing.
				pthread_cond_signal(&EOT_cond);
			}

		pthread_mutex_unlock(&EOT_mutex);

	//	printf("Voy a renderizar\n");
		// Render world
		renderSDLWorld(gameWorld, screen);
	//	printf("Termine de renderizar\n");

		// Render custom cursor
		SDL_GetMouseState(&cursorX,&cursorY);
		if(mouseClicked)
			blitSurface(screen, moveCursor,cursorX,cursorY);
		else blitSurface(screen, cursor,cursorX,cursorY);

		/* Update screen */
		SDL_Flip(screen);  

		/* Delay */
		SDL_Delay( DELAY );

	}
}

int getUserInput(SDL_World * gameWorld){

	SDL_Event event;
	Uint8 * keystate;

	double z = 1;
	int hasZoomed = 0;

	while( SDL_PollEvent( &event ) ){
            /* See if user  quits */
		if(event.type == SDL_QUIT )
			return -1;
		else if(event.type == SDL_MOUSEMOTION){
			if(mouseClicked){
				int x,y;
				SDL_GetRelativeMouseState(&x,&y);
				translateCamera(gameWorld,x,y);
			}

		}
		// I prevented calling zoom() for each event, because
		// it slows down fps. It polls way too many events,
		// so that many calls to zoom is inefficient.
		else if(event.type == SDL_MOUSEBUTTONDOWN)
			switch(event.button.button){
				case SDL_BUTTON_WHEELUP:
					z += ZOOM;
					hasZoomed = 1;
					break;
				case SDL_BUTTON_WHEELDOWN:
					z -= ZOOM;
					hasZoomed = 1;
					break;
				case SDL_BUTTON(SDL_BUTTON_LEFT):
					SDL_GetRelativeMouseState(NULL,NULL); // set postiion here
					mouseClicked = 1; // change cursor to move cursor
					break;
			}
		else if(event.type == SDL_MOUSEBUTTONUP)
			mouseClicked = 0; //change cursor here
	}
	
	if(hasZoomed)
		zoom(gameWorld,z);

	/* Get a keyboard snapshot */
	keystate = SDL_GetKeyState( NULL );

	// If user presses ESC
	if(keystate[SDLK_ESCAPE])
		return -1;
/*
	if(keystate[SDLK_DOWN])	translateCamera(gameWorld,0,-SENSITIVITY);
	if(keystate[SDLK_UP])	translateCamera(gameWorld,0,SENSITIVITY);
	if(keystate[SDLK_LEFT])	translateCamera(gameWorld,SENSITIVITY,0);
	if(keystate[SDLK_RIGHT])translateCamera(gameWorld,-SENSITIVITY,0);
*/

	return 0;
}

void shout(){
	srand((unsigned) time(NULL));
	int index = 0 + rand() % MAX_SOUNDS;
	playSound(sounds[index]);
}

void openSounds(){
	int i;
	char * path;
	if ( (path = malloc(PATH_LENGTH + 1 + EXT_LENGTH)) == NULL){
		fprintf(stderr, "%s\n", "Memory allocation error in sounds opening.");
		exit(1);
	}

	for (i=0; i<MAX_SOUNDS; i++){
		sprintf(path, "%s%d%s", SHOUT_BASE, i+1, SHOUT_EXTENSION);
		if ( (sounds[i] = malloc(sizeof(Mix_Chunk))) == NULL ){
			fprintf(stderr, "%s\n", "Memory allocation error in sounds opening.");
			exit(1);
		}
		if((sounds[i] = Mix_LoadWAV(path)) == NULL) {
			fprintf(stderr, "Unable to load WAV files: %s\n", Mix_GetError());
			exit(1);
		}
	}
}

void * playSoundOnThread(void * arg){

	int channel = (int)arg;

	while(Mix_Playing(channel) != 0);

	pthread_exit(NULL);
}


void playSound(Mix_Chunk * sound){
	int channel;
	channel = Mix_PlayChannel(CHANNEL, sound, LOOP);
	if(channel == -1) {
		fprintf(stderr, "Unable to play WAV file: %s\n", Mix_GetError());
		exit(1);
	}

	pthread_t soundT;
	pthread_create(&soundT, NULL, playSoundOnThread, (void *) channel);

}

void closeSounds(){
	int i;
	for (i=0; i<MAX_SOUNDS; i++){
		Mix_FreeChunk(sounds[i]);
	}
	
	Mix_CloseAudio();
}

