
#include <stdio.h>
#include <stdlib.h>

#define PARAM_BUFF_SIZE 64
#define NAME_BUFF_SIZE 64
#define NUMOFARGUMENTS 10
#define NUMOFCOMMANDS 10

int toExit;

typedef struct SimpleCommand{
  int numberOfAvailableArguments;
  int numberOfArguments;
  char ** arguments;
  // void SimpleCommand();
  // void insertArgument(char *argument);
}simpleCommand;


// Describes a complete command with the multiple pipes if any
// and input/output redirection if any.

typedef struct Command{
  int numberOfAvailableSimpleCommands;
  int numberOfSimpleCommands;
  struct SimpleCommand ** simpleCommands;
  char * outputFile;
  char * inputFile;
  char * errFile;
  //int background;

}command;


static struct Command *currentCommand;
static struct SimpleCommand *currentSimpleCommand;

void SimpleCommandInit(){
  currentSimpleCommand = (simpleCommand*)malloc(sizeof(simpleCommand));
  currentSimpleCommand->numberOfAvailableArguments=NUMOFARGUMENTS; // Will change when implmenting remalloc()
  currentSimpleCommand->numberOfArguments = 0;
  currentSimpleCommand->arguments = malloc(sizeof(char)*PARAM_BUFF_SIZE * NUMOFARGUMENTS);
  int i;
  for (i = 0; i < NUMOFARGUMENTS; i++)
  {
    currentSimpleCommand->arguments[i] = malloc(sizeof(char) * PARAM_BUFF_SIZE);
  }
}

void insertArgument(char* argument){
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

void prompt(){
  //TODO: make
}

void execute(){
  //TODO: make
}

void print(){
  //TODO: make
}

void clear(){
  free(currentSimpleCommand);
  free(currentCommand);
}

void insertSimpleCommand( struct SimpleCommand * simpleCommand ){
  int posOfCommands = currentCommand->numberOfSimpleCommands;
  currentCommand->simpleCommands[posOfCommands] = simpleCommand;
}

void shell_loop(){
  while(1){
   SimpleCommandInit();
   insertArgument("ls");
   printf("%s\n", currentSimpleCommand->arguments[0]);
   toExit = 1;  //Tempory Force Break until error handling
    if(toExit){
      break;
    }
  }
}

int main(int argc, char **argv){
  shell_loop();
  return 0;
}
