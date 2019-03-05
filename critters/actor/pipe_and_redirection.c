/*
 Starter code for doing piping and redirectioning
*/

#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 

void Command :: execute() {

	// save the current stdin and stdout into two new file descriptors
	// allow the end of execute() to restore them
	int tmpin = dup(0);
	int tmpout = dup(1);

	// if there is an input redirection file, then open in infile and save in fdin
	// else, create a file descriptor that refers to default input
	int fdin;
	if (infile) {
		fdin = open(infile, O_RDONLY);
	}
	else {
		fdin = dup(tmpin);
	}

	// looks through all the commands in the command table
	// create a process for every simple command and perform the pipe connections
	int ret;
	int fdout;
	for (i = 0; i < numsimplecommands; i++) {

		// redirects the standard input to come from fdin and then close it
		dup2(fdin, 0);
		close(fdin);

		// if iteration corresponds to last simple command
		// else creates a new file descriptor
		if (i == numsimplecommands - 1) {

			if (outfile) {
				fdout = open(outfile, O_WRONLY);
			}
			else {

				fdout = dup(tmpout);
			}
		}

		// for simple commands (except for the last), output is a pipe
		else {
			int fpipe[2];
			pipe(fdpipe);
			fdout = fdpipe[1];
			fdin = fdpipe[0];
		}


		// redirects standard output to go to the specified one and closes
		dup2(fdout, 1);
		close(fdout);

		// creates executable / child process if no error
		ret = fork();
		if (ret == 0) {
			execvp(scmd[i].args[0], scmd[i].args);
			perror("execvp");
			_exit(1)
		}

	}

	// restore stdin and stdout and then close the file descriptors
	dup2(tmpin, 0);
	dup2(tmpout, 1);
	close(tmpin);
	close(tmpout);

	// if not set, the parent process waits before printing out
	if(!background) {
		waitpid(ret, NULL):
	}
} 