%{
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

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
%}

%token <string_val> WORD;
%token NEWLINE GREAT LESS GREATGREAT GREATAMPERSAND PIPE AMPERSAND GREATGREATAMPERSAND NOTOKEN

%%

goal:
    command_list
    ;

arg_list:
    arg_list WORD
    | /*empty*/
           ;

cmd_and_args:
    WORD arg_list
        ;

pipe_list:
    pipe_list PIPE cmd_and_args
    | cmd_and_args
    ;

io_modifier:
    GREATGREAT WORD
    | GREAT WORD
    | GREATGREATAMPERSAND WORD
    | GREATAMPERSAND WORD
    | LESS WORD
    ;

io_modifier_list:
    io_modifier_list io_modifier
        | io_modifier
        | /*empty*/
        ;

background_optional:
    AMPERSAND {
        currentCommand-> background = 1;
    }
    | /*empty*/
    ;

command_line:
    pipe_list io_modifier_list background_opt NEWLINE
    | NEWLINE /* accept empty cmd line */
    | error NEWLINE{yyerrok;}
        /* error recovery */

command_list :
    command_line
    ; /* Command loop */

void expandWildCard(char*  arg) {
    if (strchr(arg, *) == NULL &&  strchr(arg, ?) == NULL) {
        currentSimpleCommand->insertArgument(arg);
        return;
    }

    char *reg = (char*) malloc (2*strlen(arg)+10);
    char *a = arg;
    char *r = reg;
    *r = '^' //beginning of line
    r++;

    while (*a) {
        if (*a == '*') {
            *r = '.';
            r++;
            *r='*';
            r++;
'        }
        else if (*a == '?') {
            *r = '.';
            r++;
        }
        else if (*a == '.') {
            *r = '\\';
            r++;
            *r = '.';
            r++;
        }
        else {
            *r = *a
;            r++;
        }

        a++;
    }

    //
    *r = '$';
    r++;
    *r = 0;

    regex_t re;

    // char *expbuf = (char*)malloc(strlen(reg));
    // regcomp(reg, expbuf);

    int res = regcomp(&re, reg, REG_EXTENDED|REG_NOSUB);

    if(res) {
        perror("regex compilation error");
        exit(1);
    }

    DIR * dir = opendir(".");
    if (dir == NULL) {
        perror("opening dict error");
        exit(1);
    }

    struct dirent * ent;
    while ((ent == readdir(dir)) != NULL) {
        if (regexec(ent->d_name, re) == 0) {
            currentSimpleCommand->insertArgument(strdrup(ent->d_name));
        }
    }

    closedir(dir);

}
