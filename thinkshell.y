#include <stdio.h>
#include <dirent.h>
#include <regexp.h>
#include "GridWorldShell.h"

int isWildcard = 0;

goal: command_list;

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
    GREATGREAT Word
    | GREAT Word
    | GREATGREATAMPERSAND Word
    | GREATAMPERSAND Word
    | LESS Word
    ;

io_modifier_list:
    io_modifier_list io_modifier
        | /*empty*/
        ;

background_optional:
    AMPERSAND 
    | /*empty*/
    ;

command_line:
    pipe_list io_modifier_list background_opt NEWLINE
    | NEWLINE /* accept empty cmd line */
    | error NEWLINE{yyerrok;}
        /* error recovery */

command_list :
    command_list command_line
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