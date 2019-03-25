
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
// #include <regexpr.h>
#include <pwd.h>
#include "GridWorldShell3.h"

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

int toExit = 0;
int numOfCurrentArgs;

typedef struct SimpleCommand{
  int numberOfAvailableArguments;
  int numberOfArguments;
  char ** arguments;
  // void SimpleCommand();
  // void insertArgument(char *argument);
}simpleCommand;


typedef struct Command{
  int numberOfAvailableSimpleCommands;
  int numberOfSimpleCommands;
  char ** commands;
  struct SimpleCommand ** simpleCommands;
  char * outputFile;
  char * inputFile;
  char * errFile;
  int background;

}command;


static struct Command *currentCommand;
static struct SimpleCommand *currentSimpleCommand;

// currentSimpleCommand->numberOfAvailableArguments = NUMOFARGUMENTS;
// currentSimpleCommand->numberOfArguments = 0;
// currentSimpleCommand->arguments = (char **) malloc(currentSimpleCommand->numberOfArguments * sizeof(char));;

void SimpleCommandInit(){
  currentSimpleCommand = (simpleCommand*)malloc(sizeof(simpleCommand));
  currentSimpleCommand->numberOfAvailableArguments=NUMOFARGUMENTS; // Will change when implmenting realloc()
  currentSimpleCommand->numberOfArguments = 0;
  currentSimpleCommand->arguments = malloc(sizeof(char)*PARAM_BUFF_SIZE * NUMOFARGUMENTS);
  int i;
  for (i = 0; i < NUMOFARGUMENTS; i++)
  {
    currentSimpleCommand->arguments[i] = malloc(sizeof(char) * PARAM_BUFF_SIZE);
  }
}

void insertArgument(char* argument){
  if (currentSimpleCommand->numberOfAvailableArguments == currentSimpleCommand->numberOfArguments) {
    currentSimpleCommand->numberOfAvailableArguments *= SCALEFACTOR;
    currentSimpleCommand->arguments = realloc(currentSimpleCommand->arguments, NUMOFARGUMENTS*SCALEFACTOR * sizeof(simpleCommand));
  }

  int pos = currentSimpleCommand->numberOfArguments;
  currentSimpleCommand->arguments[pos] = argument;
  currentSimpleCommand->numberOfArguments = pos+1;
}


void CommandInit(){
  currentCommand = (command*)malloc(sizeof(command));
  currentCommand->numberOfAvailableSimpleCommands = NUMOFCOMMANDS;
  currentCommand->numberOfSimpleCommands = 0;
  currentCommand->simpleCommands = malloc(sizeof(simpleCommand)* NUMOFCOMMANDS);
  int i;
  for (i = 0; i < NUMOFCOMMANDS; i++)
  {
    currentCommand->simpleCommands[i] = malloc(sizeof(simpleCommand));
  }

}

void execute(){

  int tmpin = dup(0);
  int tmpout = dup(1);

  int fdin;
  if (currentCommand->inputFile) {
    fdin = open(currentCommand->inputFile, O_RDONLY);
  }
  else {
    fdin = dup(tmpin);
  }

  int ret;
  int fdout;
  for (int i = 0; i < currentCommand->numberOfSimpleCommands; i++) {
    dup2(fdin, 0);
    close(fdin);

    if (i == currentCommand->numberOfSimpleCommands - 1) {
      if (currentCommand->outputFile) {
        fdout = open(currentCommand->outputFile, O_WRONLY);
      }
      else {
        fdout = dup(tmpout);
      }
    }
    else {
      int fdpipe[2];
      pipe(fdpipe);
      fdin = fdpipe[0];
      fdout = fdpipe[1];
    }

    dup2(fdout, 1);
    close(fdout);

    ret = fork();
    if (!ret) {
      execvp(currentSimpleCommand->arguments[0], &currentSimpleCommand->arguments[0]);
      perror(currentSimpleCommand->arguments[0]);
      exit(1);
    }
    if(!currentCommand->background){
      waitpid(ret, NULL, 0);
    }

  }


  dup2(tmpin, 0);
  dup2(tmpout, 1);
  close(tmpin);
  close(tmpout);
}

char **split_line(char *line) {
	int bufsize = LSH_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if (!tokens) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, LSH_TOK_DELIM);
	while (token != NULL) {
		tokens[position] = token;
		position++;

		if (position >= bufsize) {		// increase buffer size just in case
			bufsize += LSH_TOK_BUFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				fprintf(stderr, "lsh: allocation\n");
			}
		}

		token = strtok(NULL, LSH_TOK_DELIM);	// repeats until no token is returned
	}
	tokens[position] = NULL;
  numOfCurrentArgs = position;
	return tokens;						// array of tokens, ready to execute
}


char *read_line(void) {
	char *line = NULL;
	size_t bufsize = 0;
	getline(&line, &bufsize, stdin);
	return line;
}

void clear(){

  for (int i = 0; i < currentCommand->numberOfSimpleCommands; i++) {
    for (int j = 0; j < currentCommand->simpleCommands[i]->numberOfArguments; j++) {
      if (currentCommand->simpleCommands[i]->arguments[j]) {
        free(currentCommand->simpleCommands[i]->arguments[j]);
      }
    }
    if (currentCommand->simpleCommands[i]->arguments) {
        free(currentCommand->simpleCommands[i]->arguments);
    }
    if (currentCommand->commands[i]) {
        free(currentCommand->commands[i]);
    }
  }

  for (int i = 0; i < currentSimpleCommand->numberOfArguments; i++) {
    if (currentCommand->commands[i]) {
        free(currentCommand->commands[i]);
    }
  }
  if (currentSimpleCommand->arguments) {
        free(currentSimpleCommand->arguments);
  }

  if (currentCommand->inputFile) {
    free(currentCommand->inputFile);
  }

  if (currentCommand->outputFile) {
    free(currentCommand->outputFile);
  }

  if (currentCommand->errFile) {
    free(currentCommand->errFile);
  }

  free(currentCommand);
  free(currentSimpleCommand);
}

void insertSimpleCommand( struct SimpleCommand * simpleCommand ){
  if (currentCommand->numberOfAvailableSimpleCommands == currentCommand->numberOfSimpleCommands) {
    currentCommand->numberOfAvailableSimpleCommands *= SCALEFACTOR;
    currentCommand->simpleCommands = realloc(currentCommand->simpleCommands, NUMOFCOMMANDS * SCALEFACTOR * sizeof(simpleCommand));
  }
  int posOfCommands = currentCommand->numberOfSimpleCommands;
  currentCommand->simpleCommands[posOfCommands] = simpleCommand;
  currentCommand->numberOfSimpleCommands = currentCommand->numberOfSimpleCommands + 1;
}

void recieve_input() {
	char *line;
	char **args;
	int status;


	printf("GWS>> ");					// print prompt
	line = read_line();			// read line
	args = split_line(line);	// split line into args, an array of strings
  int i;
  for(i=0;i<numOfCurrentArgs;i++){
    insertArgument(args[i]);
  }
  if(i==1){
    insertArgument(NULL);
  }
  numOfCurrentArgs = 0;
	free(line);
	free(args);
}



void shell_loop(){
  while(1){
   SimpleCommandInit();
   CommandInit();
   recieve_input();
   insertSimpleCommand(currentSimpleCommand);
   execute();
    if(toExit){
      break;
    }
  }
}

int main(int argc, char **argv){
  shell_loop();
  return 0;
}
