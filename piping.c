#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(void) {
	int fd[2];
	pid_t childpid;

	pipe(fd);

	if ((childpid = fork()) == -1) {
		perror("fork");
		exit(1);
	}
	else if (childpid == 0) {
		close(fd[0]);
	}
	else
		close(fd[1]);

	return 0;
}