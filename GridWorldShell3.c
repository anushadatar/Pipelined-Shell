
#include <stdio.h>
#include <stdlib.h>

#define PARAM_BUFF_SIZE 64
#define NAME_BUFF_SIZE 64
#define NUMOFARGUMENTS 10

int toExit;

typedef struct SimpleCommand{
  int numberOfAvailableArguments;
  int numberOfArguments;
  char ** arguments;
  // void SimpleCommand();
  // void insertArgument(char *argument);
}SimpleCommand;


// Describes a complete command with the multiple pipes if any
// and input/output redirection if any.

typedef struct Command{
  int numberOfAvailableSimpleCommands;
  int numberOfSimpleCommands;
  SimpleCommand ** simpleCommands;
  char * outputFile;
  char * inputFile;
  char * errFile;
  int background;
  // void prompt();

  // void print();
  // void execute();
  // void clear();
  // Command();
  // void insertSimpleCommand( SimpleCommand * simpleCommand );

}Command;


static struct Command currentCommand;
static struct SimpleCommand *currentSimpleCommand;

void SimpleCommandInit(){
  currentSimpleCommand = (SimpleCommand*)malloc(sizeof(SimpleCommand));
  currentSimpleCommand->numberOfAvailableArguments=NUMOFARGUMENTS; // Will change when implmenting remalloc()
  currentSimpleCommand->numberOfArguments = 0;
  currentSimpleCommand->arguments = malloc(sizeof(char)*PARAM_BUFF_SIZE * NUMOFARGUMENTS);
  int i;
  for (i = 0; i < NUMOFARGUMENTS; i++)
  {
    currentSimpleCommand->arguments[i] = malloc(sizeof(char) * PARAM_BUFF_SIZE);
  }
  // currentSimpleCommand->arguments = (char*)malloc(sizeof(char)*PARAM_BUFF_SIZE);
}

void insertArgument(char* argument){
  int pos = currentSimpleCommand->numberOfArguments;
  currentSimpleCommand->arguments[pos] = argument;
  currentSimpleCommand->numberOfArguments = pos+1;
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
