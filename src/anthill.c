#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h> //signal
#include <stdio.h>	//printf
#include <string.h> //memcmp
#include <stdlib.h>	// exit

void endOfFifo(int n);
void fatal(char *s);
int anthill(int testAnts);

int main(void){
	anthill(7);
}

int anthill(int testAnts){
	int pid, status, parentPid;
	int fd, n;
	char buffer[200];
	char * fifoName = "/tmp/fifo";

	/** Variables for 'Send Random string' TEST **/
	int index, length = 5;
	char * stringSource[] = {"dead boys", "motosierra", "hellacopters", "stooges", "new york dolls"};
	/**********************************************/

	if ( access(fifoName, 0) == -1 && mknod(fifoName, S_IFIFO | 0666, 0) == -1 ){
		fatal("FIFO could not be created");
	}

	while( testAnts > 0){
		if ( !(pid = fork()) ){ /* Ant process (child) */

			fd = open(fifoName, O_WRONLY);

			/* 'Read from stdin' TEST *//*
			n = read(0, buffer, sizeof buffer);
			printf("Hormiga %d mandó: %.*s", getpid(), n, buffer);
			write(fd, buffer, n); */

			/* 'Send random string' TEST */
			int index = getpid() % length;
			n = strlen(stringSource[index]);
			printf("Hormiga %d mandó: %.*s\n", getpid(), n, stringSource[index]);
			write(fd, stringSource[index], n);
			return;

		} else if (pid > 0){	/* Anthill process (parent) */
			/* Anthill and ants maintain contact through 'anthillFifo' FIFO */
			signal(SIGPIPE, endOfFifo);
			fd = open(fifoName, O_RDONLY);
			wait(&status);
			n = read(fd, buffer, sizeof buffer);
			printf("Hormiguero recibió: %.*s\n", n, buffer);

		} else {
			fatal("FORK could not be done");
		}
		testAnts--;
	}
}

void fatal(char *s){
	perror(s);
	exit(1);
}

void endOfFifo(int n){
	printf("Ejecución terminada");
	exit(1);
}
