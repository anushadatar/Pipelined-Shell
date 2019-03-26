/* Template for one potential easter egg - a lisp interpreter.
	Toggled by "lisp"
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

int opsub(char* input) {
	int i = 0;
	int j = 0;

	while(*(++input) == ' ');

	sscanf(input, "%d", &i);

	while(*(++input) == ' ');

	sscanf(input, "%d", &j);

	return (i - j);

}

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

int opdiv(char *input) {
	int i = 0;
	int j = 0;

	while(*(++input) == ' ');

	sscanf(input, "%d", &i);

	while(*(++input) == ' ');

	sscanf(input, "%d", &j);

	return (i / j);

}

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

// int main() {
// 	char* input = malloc(1024);
// 	printf("Enter command.\n");

// 	while(1) {
// 		fgets(input, sizeof(input), stdin);

// 		if (strcmp(input, "lisp\n") == 0) {

// 			while(1) {
// 				free(input);
// 				input = malloc(1024);
// 				readlisp(input, 1024);
// 				printf("-> %d\n", evaluate(input));
// 			}
// 		}
// 		else {
// 			break;
// 		}	
// 	}

// 	return 0;
// }