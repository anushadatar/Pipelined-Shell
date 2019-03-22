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
			sscanf(input, "%d", &i);
			result += i;
			input++;
		}
	}

	return result;
}

int opsub(char *input) {
	int i = 0;
	int j = 0;

	while (*input == ' ') {
		input++;
	}

	sscanf(input, "%d", &i);

	while (*input == ' ') {
		input++;
	}

	sscanf(input, "%d", &j);

	return (i - j);

}

int opprod(char* input) {
	int result = 0;

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

	while (*input == ' ') {
		input++;
	}

	sscanf(input, "%d", &i);

	while (*input == ' ') {
		input++;
	}

	sscanf(input, "%d", &j);

	return (i / j);

}

int evaluate(char* input) {

	if (*input != '(') {
		printf("%c", *input);
		printf("%s", input);
		fprintf(stderr, "invalid lisp expression\n");
		exit(1);
	}

	input++;
	char *op = *input;
	input++;

	int result = 0;

	// switch (op) {
	// 	case ADD:
	// 		result = opadd(input);
	// 		break;
	// 	case SUB:
	// 		result = opsub(input);
	// 		break;
	// 	case MUL:
	// 		result = opprod(input);
	// 		break;
	// 	case DIV:
	// 		result = opdiv(input);
	// 		break;
	// 	default:
	// 		break;
	// }

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

int main() {
	char* input = malloc(256);
	printf("Enter command.\n");

	while(1) {
		fgets(input, sizeof(input), stdin);

		if (strcmp(input, "lisp\n") == 0) {

			while(1) {
				free(input);
				input = malloc(256);
				fgets(input, sizeof(input), stdin);
				// printf("%s", input);
				// printf("The result is: ");
				printf("-> %d\n", evaluate(input));
			}
		}
		else {
			break;
		}	
	}

	return 0;
}