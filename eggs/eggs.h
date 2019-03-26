#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define START '('
#define ADD '+'
#define SUB '-'
#define MUL '*'
#define DIV '/'

#define THINKOS "http://greenteapress.com/thinkos/thinkos.pdf"
#define THINKPERL "http://greenteapress.com/wp/think-perl-6/"
#define THINKSTATS "http://greenteapress.com/thinkstats2/thinkstats2.pdf"
#define THINKBAYES "http://www.greenteapress.com/thinkbayes/thinkbayes.pdf"
#define THINKDSP "http://greenteapress.com/thinkdsp/thinkdsp.pdf"
#define THINKJAVA "http://greenteapress.com/thinkjava6/thinkjava.pdf"
#define THINKDS "http://greenteapress.com/thinkdast/thinkdast.pdf"
#define THINKCOM "http://greenteapress.com/complexity2/thinkcomplexity2.pdf"
#define THINKPY "http://greenteapress.com/thinkpython2/thinkpython2.pdf"
#define ALLEN "http://www.allendowney.com/wp/"

int opadd(char* input);
int opsub(char* input);
int opprod(char* input);
int opdiv(char* input);
int evaluate(char* input);
void readlisp(char* string, int length);

void readin(char* string, int length);
void choose(char *input, char *url);

void printTitle();
void printGrid();
void printCritter();
void printegg(char** input);