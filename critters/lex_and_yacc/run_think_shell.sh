lex thinkshell.l
yacc -dy thinkshell.y
gcc lex.yy.c GridWorldShell3.c y.tab.c -o shell
