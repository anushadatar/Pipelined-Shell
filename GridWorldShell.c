
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
#include "GridWorldShell.h"
#include "eggs/eggs.h"
// #include "eggs/quark.h"

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

// forward declarations for built in shell commands
void lsh_cd(char **args);
void lsh_mkdir(char **args);
void print(int t);
// void readlisp(char* string, int length);
void teachme(char ** input);
void printegg(char** input);
void lisp(char** input);
void notvim(char** input);


// build in commands - array
char *builtin_str[] = {
  "cd",
  "mkdir",
  "lisp",
  "teach",
  "grid",
  "title",
  "critter"
  // "vim"
};

// All built in function calls
void (*builtin_funct[]) (char **) = {
  &lsh_cd,
  &lsh_mkdir,
  &lisp,
  &teachme,
  &printegg,
  &printegg,
  &printegg
};

int toExit = 0; // Status variable that signals to exit program
int numOfCurrentArgs; // Parsing track variable of current user intput arguments

// Simple command struct that holds user input arguments
typedef struct SimpleCommand{
  int numberOfAvailableArguments;
  int numberOfArguments;
  char ** arguments;
}simpleCommand;

// Command struct that holds simple commands and file handeling
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

// Instances of global command and simple command current being used
static struct Command *currentCommand;
static struct SimpleCommand *currentSimpleCommand;


//// Easter Egg function keyword calls

// Calls print easter egg
void printegg(char** input){
  if(!strcmp(input[0],"grid")){
    printGrid();
  }else if(!strcmp(input[0],"title")){
    printTitle();
  } else if(!strcmp(input[0], "critter")){
    printCritter();
  }
}

// Sets up teach me easter egg with user input
void teachme(char **input) {
   char *url = malloc(60 + sizeof(char) * strlen("xdg-open "));
   readin(input[0], 2048);
   strcpy(url, "xdg-open ");
   choose(input[0], url);
   system(url);
}

// Allocates space and calls lisp easter egg
void lisp(char **input) {
  char * in = malloc(1024);
  readlisp(in, 1024);
  printf("-> %d\n", evaluate(in));
}

// Returns the number of build in functions

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

//// Built in function calls

void lsh_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument \"cd\"\n");
  }
  else if (chdir(args[1]) != 0) {
    perror("lsh");
  }
  printf("here %s\n", args[1]);
}

void lsh_mkdir(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument \"mkdir\"\n");
  }
  else if ((mkdir(args[1],00777)) == -1) {
    perror("lsh");
  }
}

/*  Intializes simpleCommand

  Allocates space for simple command and its arguments
  Sets to global currentSimpleCommand

*/

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


/*  insertArgument: inserts argument into currentSimpleCommand

  Called for every delimiter seperated user command
  Adds argument to the current simple command
  Increments number of arguments in currentSimpleCommand

  argument: pointer to character array
*/

// Takes a user argument and add it to the current simple command, increments number of commands
void insertArgument(char* argument){
  if (currentSimpleCommand->numberOfAvailableArguments == currentSimpleCommand->numberOfArguments) {
    currentSimpleCommand->numberOfAvailableArguments *= SCALEFACTOR;
    currentSimpleCommand->arguments = realloc(currentSimpleCommand->arguments, NUMOFARGUMENTS*SCALEFACTOR * sizeof(simpleCommand));
  }

  int pos = currentSimpleCommand->numberOfArguments;
  currentSimpleCommand->arguments[pos] = argument;
  currentSimpleCommand->numberOfArguments = pos+1;
}

/*  Intializes Command

  Allocates space for command and its simple commands
  Sets to global currentCommand

*/

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

/* Executes commands stored in the currentCommand

  Sets up input and output of currentCommand, if no location specified, defaults to standard output/input
  Forks a child process for each user argument command

*/
void execute(){

  for (int i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(currentSimpleCommand->arguments[0], builtin_str[i]) == 0) {
      (*builtin_funct[i])(currentCommand->simpleCommands[0]->arguments);
      return;
    }
  }

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

/* split_line: returns char 2d array of user input commands

  Takes pointer to character array

  Returns a pointer an array of pointers to user input

  *line: pointer to character array

*/

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

/*  read_line: returns char pointer to user's input

  Retrives user's input line with getline() function call
  Returns a pointer to the character array of the retrieved line

*/


char *read_line(void) {
	char *line = NULL;
	size_t bufsize = 0;
	getline(&line, &bufsize, stdin);
	return line;
}

/*  print: prints current user commands

  Iterates through and prints all simple commands
  Iterates through and prints all commands

*/

void print(int t) {
  if (t == 0) {
    printf("\nSimple Command Table:\n");

    for (int i = 0; i < currentSimpleCommand->numberOfArguments; i++) {
      printf("%s\t", currentSimpleCommand->arguments[i]);
    }

    printf("\n");
  }
  else {
    printf("\nCommand Table:\n");

    for (int i = 0; i < currentCommand->numberOfSimpleCommands; i++) {
      for (int j = 0; j < currentCommand->simpleCommands[i]->numberOfArguments; j++) {
        printf("%s\t", currentCommand->simpleCommands[i]->arguments[j]);
      }
      printf("\n");
    }
  }
}

/*  clear: frees memory stored in the currentCommand and currentSimpleCommand

  Frees all memory of currentSimpleCommand arguments
  Frees all memory of currentCommand simpleCommands

*/

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

/*  insertSimpleCommand: inserts populated simple command into the currentCommand

  Checks if more space is needed for the currentSimpleCommand
  Added currentSimpleCommand to currentCommand

*/

void insertSimpleCommand( struct SimpleCommand * simpleCommand ){
  if (currentCommand->numberOfAvailableSimpleCommands == currentCommand->numberOfSimpleCommands) {
    currentCommand->numberOfAvailableSimpleCommands *= SCALEFACTOR;
    currentCommand->simpleCommands = realloc(currentCommand->simpleCommands, NUMOFCOMMANDS * SCALEFACTOR * sizeof(simpleCommand));
  }
  int posOfCommands = currentCommand->numberOfSimpleCommands;
  currentCommand->simpleCommands[posOfCommands] = simpleCommand;
  currentCommand->numberOfSimpleCommands = currentCommand->numberOfSimpleCommands + 1;

}

/*  Revieve_input: pares user input

  Calls read_line function to retrieve the user's input string
  Breaks the line up by spaces the delimiters: LSH_TOK_DELIM
  Populates simpleCommand's argument from user's inputs

*/

void recieve_input() {

	char *line;
	char **args;
	int status;

	line = read_line();
	args = split_line(line);
  int i;
  for(i=0;i<numOfCurrentArgs;i++){
    insertArgument(args[i]);
  }
  if(i==1){
    insertArgument(NULL);
  }
  numOfCurrentArgs = 0;
}

/*  shell_loop loops through calls to parse and execute user input

  Creates the curentSimpleCommand and currentCommand
  Calls parse of user input to populate currentSimpleCommand
  Calls to execute command
  Checks for a call to exit

*/

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
