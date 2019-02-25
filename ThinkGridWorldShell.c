#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define INPUT_BUFF_SIZE 1024;
#define PARAM_BUFF_SIZE 64;

int toExit = 0;

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
      token = strtok(NULL," ");
      i++;
  }
  paramsList[i]=NULL;
  return paramsList;
}

/*
  Reads and stores the user's input from stdin into buffer

*/
char* readLine(){
  size_t size = INPUT_BUFF_SIZE; //automatically allocated with getline
  char *inputBuff = (char *)malloc(size * sizeof(char));
  printf(">>> ");
  getline(&inputBuff,&size,stdin);
  return inputBuff;
}

/*
  Loops through reading and parsing lines until exit command is reached

*/
void shell_lop(){
  while(1){
    char *inputLine = readLine();
    char **inputParams = parseParams(inputLine);
    toExit = 1;  //Tempory Force Break until error handling
    if(toExit){
      break;
    }
  }
}

int main(int argc, char **argv){
  shell_lop();
  return 0;
}
