#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUF_SIZE 100

int child_task(int read_pipe) {
	char to_msg1[BUF_SIZE];
	char to_msg2[BUF_SIZE];
	int nr;

	if ((nr = read(read_pipe, to_msg1, BUF_SIZE)) == -1) {
		perror("child failed to read from pipe");
		return 1;
	}
	printf("read from pipe\n");
	printf("bytes: %d\n", nr);


	if ((nr = read(read_pipe, to_msg2, BUF_SIZE)) == -1) {
		perror("child failed to read from pipe");
		return 1;
	}
	printf("read from pipe\n");
	printf("bytes: %d\n", nr);
	printf("child received message from parent.\n");
	printf("messages received: \"%s\" and \"%s\".\n", to_msg1, to_msg2);

	return 0;
}

int parent_task(int write_pipe) {
	int nw;
	char from_msg1[] = "Thank you for the money!";
	char from_msg2[] = "Wish you were here in Paris with me";

	//send the first message
	if ((nw = write(write_pipe, from_msg1, strlen(from_msg1) + 1)) == -1) {
		perror("parent failed to write to pipe");
		exit(1);
	}
	printf("wrote to the pipe 1\n");
	printf("bytes written: %d\n", nw);
	//send the second message
	if ((nw = write(write_pipe, from_msg2, strlen(from_msg2) + 1)) == -1) {
		perror("parent failed to write to pipe");
		exit(1);
	}
	printf("wrote to the pipe 2\n");

	printf("parent sent messages to child.\n");
	printf("messages sent: \"%s\" and \"%s\".\n", from_msg1, from_msg2);
	printf("bytes written: %d\n", nw);

	return 0;
}

int main() {
	int parent_to_child_pipe[2];

	// Create the pipe
	if (pipe(parent_to_child_pipe) == -1) {
		perror("could not create pipe");
		exit(1);
	}

	pid_t pid;
	if ((pid = fork()) == 0) {
		// Child process...
		// First we close the write end of the pipe.
		close(parent_to_child_pipe[1]);



		exit(child_task(parent_to_child_pipe[0]));
	} else {
		int exit_condition;
		// Parent process...
		// First, we close the read end of the pipe.
		close(parent_to_child_pipe[0]);
		exit_condition = parent_task(parent_to_child_pipe[1]);

		// Wait on child process to complete
		wait(NULL);
		printf("parent process completing.\n");

		exit(exit_condition);
	}
}
