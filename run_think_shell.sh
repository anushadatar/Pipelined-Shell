lex thinkshell.l
yacc -y thinkshell.y
gcc lex.yy.c y.tab.c
