/* Header file for GridWorldShell3.c
Defines command structs
*/

#ifndef GRIDWORLDSHELL3_H
#define GRIDWORLDSHELL3_H

#define PARAM_BUFF_SIZE 64
#define NAME_BUFF_SIZE 64
#define NUMOFARGUMENTS 10
#define NUMOFCOMMANDS 10
#define SCALEFACTOR 2

#include <stdio.h>
#include <stdlib.h>

extern int toExit;

typedef struct SimpleCommand{
    int numberOfAvailableArguments;
    int numberOfArguments;
    char ** arguments; 
//    void SimpleCommand();
//    void insertArgument(char *argument);
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

static struct Command* currentCommand;
static struct SimpleCommand* currentSimpleCommand;
void insertSimpleCommand(struct SimpleCommand* simpleCommand);
void SimpleCommandInit(void);
void insertArgument(char* argument);
void CommandInit(void);
void execute(void);



#endif
