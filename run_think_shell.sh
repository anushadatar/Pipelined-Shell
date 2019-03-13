lex thinkshell.l
yacc -dy thinkshell.y
gcc lex.yy.c y.tab.c -o shell
