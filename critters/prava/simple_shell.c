/**
Simple shell implemented in C.

Followed Stephan Brennan's Write a Shell in C tutorial.

**/

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

// forward declarations for built in shell commands
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

// built in commands - array
char *builtin_str[] = {
	"cd",
	"help",
	"exit"
};

// commands corresponding functions
int (*builtin_funct[]) (char **) = {
	&lsh_cd,
	&lsh_help,
	&lsh_exit
};

int lsh_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}

// builtin function implementations
// lsh_cd checks if second arg exists and errors if not
int lsh_cd(char **args) {
	if (args[1] == NULL) {
		fprintf(stderr, "lsh: expected argument \"cd\"\n");
	}
	else {
		if (chdir(args[1]) != 0) {
			perror("lsh");
		}
	}
	return 1;
}

int lsh_help(char **args) {
	printf("Allen Downey's LSH \n");
	printf("Type program names and arguments, and hit enter \n");
	printf("The following are build in:\n");

	for (int i = 0; i < lsh_num_builtins(); i++) {
		printf("	%s\n", builtin_str[i]);
	}

	printf("Use the man command for information on other programs\n");
	return 1;
}


int lsh_exit(char **args) {
	return 0;
}

char *lsh_read_line(void) {
	char *line = NULL;
	ssize_t bufsize = 0;
	getline(&line, &bufsize, stdin);
	return line;
}

// like lsh_read_line(), but having a null_terminated array of pointers and not chars
char **lsh_split_line(char *line) {
	int bufsize = LSH_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if (!tokens) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, LSH_TOK_DELIM);
	while (token != NULL) {
		tokens[position] = token;
		position++;

		if (position >= bufsize) {		// increase buffer size just in case
			bufsize += LSH_TOK_BUFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				fprintf(stderr, "lsh: allocation\n");
			}
		}

		token = strtok(NULL, LSH_TOK_DELIM);	// repeats until no token is returned
	}
	tokens[position] = NULL;
	return tokens;						// array of tokens, ready to execute
}


// takes list of arguments from earlier
int lsh_launch(char **args) {
	pid_t pid, wpid;
	int status;

	pid = fork();						// forks the process
	if (pid == 0) {						// want to run command run by user
		// Child process
		if (execvp(args[0], args) == -1) {	// variant of exec
			perror("lsh");				// error and exit so shell can run
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0) {					// if fork() had an error
		// Error forking
		perror("lsh");
	}
	else {								// successful execution, know child is going to execute process
		// Parent process
		do {
			wpid = waitpid(pid, &status, WUNTRACED);	// exit or killed with signal
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

int lsh_execute(char **args) {
	if (args[0] == NULL) {
		// empty command entered
		return 1;
	}

	for (int i = 0; i < lsh_num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_funct[i])(args);
		}
	}

	return lsh_launch(args);
}

// this entire function is arbitrary and can be replaced with a getline()
char *lsh_read_line_null(void) {
	int bufsize = LSH_RL_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;

	if (!buffer) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		// Read a character
		c = getchar();					// store in int b/c EOF is int

		// If we hit EOF, replace it with a null char and return
		if (c == EOF || c == '\n') {
			buffer[position] = '\0';
			return buffer;
		}
		else {
			buffer[position] = c;
		}
		position++;

		// if we have exceed buffer, reallocate
		if (position >= bufsize) {
			bufsize += LSH_RL_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			if (!buffer) {
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

void lsh_loop(void) {
	char *line;
	char **args;
	int status;

	do {
		printf("> ");					// print prompt
		line = lsh_read_line();			// read line
		args = lsh_split_line(line);	// split line into args
		status = lsh_execute(args);		// execute args

		free(line);
		free(args);
	} while (status);					// use status to determine when to exit
}


int main(int argc, char **argv) {

	// load configuration files if there are any

	// run command loop
	lsh_loop();

	// perform any shutdown/cleanup

	return EXIT_SUCCESS;
}