/* thinkshell.l : A lexical analyzer for the shell. */

/* This code goes directly into the file. */

%{ 
#include <string.h>
#include "y.tab.h" 

extern YYSTYPE yylval;

/* Global variables can live here. */
%}

%% 
\n { 
return NEWLINE; 
}

[ \t] { 
    /* Discard spaces and tabs */ 
}

">" { 
    return GREAT; 
}

"<" { 
    return LESS; 
}

">>" { 
    return GREATGREAT;
}

">&" { 
    return GREATAMPERSAND; 
}

"|" { 
    return PIPE; 
}

"&" { 
    return AMPERSAND; 
}

[^ \t\n\] [^ \t\n]* {
    /* Assume that filenames only have alphanumeric characters. */
    yylval.string_val = strdup(yytext);
    return WORD;
}

. {
    /* Invalid character included in input. */
    return NOTOKEN;
  }

%%
