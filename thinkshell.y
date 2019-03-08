%{

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
        | /*empty*/
        ;

background_opt:
    AMPERSAND 
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
%%
