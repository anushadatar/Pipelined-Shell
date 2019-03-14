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
/*
int yywrap() 
{
    return 1;
}
*/

int main()
{
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
    }
    | NEWLINE
    | error NEWLINE {
        yyerrok;
    }
    ;

command_and_args:
    command_word arg_list {
        // TODO Insert simple command.
        // or expand wildcards if necessary.
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
    WORD {
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
    | 
    ;

io_modifier_list:
    io_modifier_list io_modifier
        | /*empty*/
        ;

background_opt:
    AMPERSAND {
        // Change background on simplecommand
    }
    | /*empty*/
    ;

io_modifier_opt:
    GREAT WORD {
    
    }
    | GREATGREAT WORD {

    }
    | GREATGREATAMPERSAND WORD {
    }
    | GREATAMPERSAND WORD {
    }
    | LESS WORD {
    }
;
%%
