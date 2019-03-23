%{
#include <stdio.h>
#include <stdlib.h>
#include "GridWorldShell3.h"
#include "util.h"
#include "y.tab.h"

int yylex();
int yyparse();

void yyerror(const char *str)
{
    fprintf(stderr, "error: %s\n", str);
}

int main() {
    yyparse();
}
%}

%token <string_val> WORD;
%token NEWLINE GREAT LESS GREATGREAT GREATAMPERSAND PIPE AMPERSAND GREATGREATAMPERSAND NOTOKEN


%%
goal: 
    command_list
    ;

command_list:
    command_list command
    | command
;

command:
    simple_command
    |    
  ; 

simple_command:
    pipe_list io_modifier_list background_opt NEWLINE {
       // TODO Code to execute current command.
        execute();
    }
    | NEWLINE
    | error NEWLINE {
        yyerrok;
    }
    ;

command_and_args:
    command_word arg_list {
        // TODO Insert simple command.
        SimpleCommandInit();
        insertSimpleCommand(currentSimpleCommand); 
    }
    ;

arg_list:
    arg_list argument
    |
    ;

argument:
    WORD
    ;

command_word:
    WORD 
    {
   //     SimpleCommandInit();
        insertArgument( $1 );    
        // New simple command
        // Insert arguments from shell
    }
    ;
  
pipe_list:
    pipe_list PIPE command_and_args
    | command_and_args
    ;

io_modifier:
    io_modifier_list io_modifier_opt
    | io_modifier_opt
    ;

io_modifier_list:
    io_modifier_list io_modifier
        | /*empty*/
        ;

background_opt:
    AMPERSAND {
        // Change background on simplecommand
    }
    ;

io_modifier_opt:
    GREAT WORD {
        currentCommand->outputFile = $2;
    
    }
    | GREATGREAT WORD {
        // TODO This needs to append if possible.
        currentCommand->outputFile = $2;
    }
    | GREATGREATAMPERSAND WORD {
   
     }
    | GREATAMPERSAND WORD {
    }
    | LESS WORD {
    }
;
%%
/*
#include <dirent.h>
#include <regex.h>
#include <string.h>
void expandWildCard(char*  arg) {
    if ((strchr(arg, '*') == NULL) &&  (strchr(arg, '?') == NULL)) {
        insertArgument(arg);
        return;
    }

    char *reg = (char*) malloc (2*strlen(arg)+10);
    char *a = arg;
    char *r = reg;
    *r = '^'; //beginning of line
    r++;

    while (*a) {
        if (*a == '*') {
            *r = '.';
            r++;
            *r='*';
            r++;
        }
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
            insertArgument(strdrup(ent->d_name));
        }
    }

    closedir(dir);

}

*/
