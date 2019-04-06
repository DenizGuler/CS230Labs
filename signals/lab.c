#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_CHILDREN 4

int cum_sum;

void worker_handler(int signo, siginfo_t *info, void *extra) {
	cum_sum += info->si_value.sival_int;
	printf("Worker received signal:  %d, value: %d\n", signo, cum_sum);
}

int worker() {
	// Get parent PID
	pid_t parent_pid = getppid();
	pid_t pid = getpid();

	// Declare a sigaction structure and set its sa_flags
	// field to SA_SIGINFO to allow us to send data as part
	// of the signal and its sa_sigaction to the address of
	// the signal handler function above.
	struct sigaction action;
	action.sa_flags = SA_SIGINFO;
	action.sa_sigaction = &worker_handler;

	// Install the signal handler to be called when this
	// process receives a SIGUSR1 signal.
	if (sigaction(SIGUSR1, &action, NULL) == -1) {
		perror("sigusr: sigaction");
		return 1;
	}

	printf("Signal handler installed, waiting for signal\n");

	while(1) {
		sleep(2);
		union sigval value;
		value.sival_int = 9;

		if(sigqueue(parent_pid, SIGUSR1, value) == 0) {
			printf("signal sent successfully!!\n");
		} else {
			printf("oi cheeky cunt mate\n");
			perror("SIGSENT-ERROR:");
		}
	}
}

void manager_handler(int signo, siginfo_t *info, void *extra) {
	cum_sum += info->si_value.sival_int;
	printf("Manager received signal: %d, sum: %d\n", signo, cum_sum);
}

int manager(int child_pid[NUM_CHILDREN]) {
	// Declare a sigaction structure and set its sa_flags
	// field to SA_SIGINFO to allow us to send data as part
	// of the signal and its sa_sigaction to the address of
	// the signal handler function above.
	struct sigaction action;
	action.sa_flags = SA_SIGINFO;
	action.sa_sigaction = &manager_handler;

	// Install the signal handler to be called when this
	// process receives a SIGUSR1 signal.
	if (sigaction(SIGUSR1, &action, NULL) == -1) {
		perror("sigusr: sigaction");
		return 1;
	}

	printf("Signal handler installed, waiting for signal\n");

	while(1) {
		sleep(6);
		union sigval value;
		value.sival_int = 4;
		// signal delivered successfully to child
		for (int i = 0; i < NUM_CHILDREN; ++i) {
			if (sigqueue(child_pid[i], SIGUSR1, value) == 0) {
				printf("signal sent successfully to child!!\n");
			} else {
				printf("wtf is happening\n");
				perror("SIGSENT-ERROR:");
			}
		}
	}
}

int main() {
	int child_pids[NUM_CHILDREN];
	int pid = fork();
	if (pid != 0) {
		child_pids[0] = pid;
	}
	// parent process
	for(int i = 1; i < NUM_CHILDREN; ++i) {
		if(pid != 0) {
			pid = fork();
			child_pids[i] = pid;
			// generating children to store in the array
		}
	}
	if (pid == 0) {
		if (worker() >= 1) {
			printf("worker failed");
			return 1;
		}
	} else {
		if (manager(child_pids) >= 1) {
			printf("manager failed");
			wait(NULL);
			return 0;
		}
	}

	return 0;
}
