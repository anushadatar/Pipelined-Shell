/* Easter Egg! Rudimentary lisp interpreter.
*  Performs add, subtract, multiplication, and division operations.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define START '('
#define ADD '+'
#define SUB '-'
#define MUL '*'
#define DIV '/'

/* Function to add a series of numbers together.
*  input: string input of format "+ {num} {num} ..."
*  output: result of the operation
*/
int opadd(char* input) {
	int result = 0;

	while (*input != ')') {
		if (*input == ' ') {
			input++;
		}
		else {
			int i = 0;
			sscanf(input++, "%d", &i);
			result += i;
		}
	}

	return result;
}

/* Function to subtract two numbers.
*  input: string input of format "- {num} {num}"
*  output: result of the operation
*/
int opsub(char* input) {
	int i = 0;
	int j = 0;

	while(*(++input) == ' ');

	sscanf(input, "%d", &i);

	while(*(++input) == ' ');

	sscanf(input, "%d", &j);

	return (i - j);

}

/* Function to multiply a series of numbers together.
*  input: string input of format "* {num} {num} ..."
*  output: result of the operation
*/
int opprod(char* input) {
	int result = 1;

	while (*input != ')') {
		if (*input == ' ') {
			input++;
		}
		else {
			int i = 0;
			sscanf(input++, "%d", &i);
			result *= i;
		}
	}

	return result;
}

/* Function to divide two numbers.
*  input: string input of format "/ {num} {num}"
*  output: result of the operation
*/
int opdiv(char *input) {
	int i = 0;
	int j = 0;

	while(*(++input) == ' ');

	sscanf(input, "%d", &i);

	while(*(++input) == ' ');

	sscanf(input, "%d", &j);

	return (i / j);

}

/* Function to evaluate the input, choose the correct operation,
*    and returns the result.
*  input: string input of format "{op} {num} ..."
*  output: returns result of the operation
*/
int evaluate(char* input) {


	if (*(input++) != '(') {
		return 0;
	}

	char *op = *(input++);

	int result = 0;

	if (op == '+') {
		result = opadd(input);
	}
	else if (op == '-') {
		result = opsub(input);
	}
	else if (op == '*') {
		result = opprod(input);
	}
	else if (op == '/') {
		result = opdiv(input);
	}

	return result;
}

/* Takes an input of a string and a length and gets input/
*/ 
void readlisp(char* string, int length)
{
    int x;

    fgets(string,length,stdin);
    for(x=0;x<=length;x++)
    {
        if( string[x] == '\n')
        {
            string[x] = '\0';
            break;
        }
    }
}