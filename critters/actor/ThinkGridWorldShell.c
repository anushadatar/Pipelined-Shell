#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ThinkGridWorldShell.h"

#define INPUT_BUFF_SIZE 1024;
#define PARAM_BUFF_SIZE 64;
#define NUM_BUILTINS 1;

int toExit = 0;

/*
*/
char *BUILTIN_COMMANDS[] = {"cd"};
/*
*/
int (*BUILTIN_COMMAND_FUNCTIONS[]) (char**) = {
     &shell_cd
};

int shell_cd(char **inputParams) {
    if (inputParams[1] == NULL) {
        perror("shell");
    } else {
        if (chdir(inputParams[1]) != 0) {
            perror("shell");
        }
    }
    return 1;
}

/*

*/
int execute(char **inputParams)
{
    int i;
    for (i = 0; i < 1; i++) {
        if (strcmp(inputParams[0], BUILTIN_COMMANDS[i]) == 0) {
            return (*BUILTIN_COMMAND_FUNCTIONS[i])(inputParams);
        }
    }
    /* If it is not a built-in command, go ahead and execute. */
    pid_t process, waitProcess;
    int process_state;
    printf("Kicking off process");
    process = fork();
    if (process == 0) {
        if (execvp(inputParams[0], inputParams) == -1) {
            perror("shell");

        }
        exit(EXIT_FAILURE);
    } else if (process < 0) {
        perror("shell");
    } else {
        do {
            waitProcess = waitpid(process, &process_state, WUNTRACED);
        } while (!WIFEXITED(process_state) && !WIFSIGNALED(process_state)); 
    }
    return 1;
}
/*
  Reads and seperates user parameters on a single line

  *inputLine: reference to line to be parsed
*/

char** parseParams(char *inputLine){
  size_t size = PARAM_BUFF_SIZE;
  char **paramsList = (char **)malloc(size * sizeof(char));
  char *token;
  token = strtok(inputLine," "); //Current seperator set by spaces
  int i=0;
  while(token!=NULL){
      paramsList[i]= token;
      i++;
      token = strtok(NULL," ");
  }
  paramsList[i]=NULL;
  return paramsList;
}

/*
  Reads and stores the user's input from stdin into buffer

*/
char* readLine(){
  size_t size = 0; //INPUT_BUFF_SIZE; //automatically allocated with getline
//char *inputBuff = (char *)malloc(size * sizeof(char));
  char* inputBuff = NULL;
  printf(">>> ");
  getline(&inputBuff,&size,stdin);
  return inputBuff;
}

/*
  Loops through reading and parsing lines until exit command is reached

*/
void shell_loop(){
  while(1){
    char *inputLine = readLine();
    char **inputParams = parseParams(inputLine);
    int status = execute(inputParams);
   /* toExit = 1;  //Tempory Force Break until error handling
    if(toExit){
      break;
    }*/
  }
}

int main(int argc, char **argv){
  shell_loop();
  return 0;
}
