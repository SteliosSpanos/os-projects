#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void handler_function(int sig);

int main() {
	pid_t pid;
	int status;
	char *args[] = {"/bin/ls", "-l", ".", NULL};

	pid = fork();
	if (pid < 0) {
		perror("fork");
		return 1;
	}
	else if (pid == 0) {
		printf("[Child]: PID: %d\n", getpid());
		if (execv(args[0], args) < 0) {
			perror("exec");
			return 1;
		}
	}
	else {
		waitpid(pid, &status, 0);

		struct sigaction act;
		act.sa_handler = handler_function;
		sigaction(SIGINT, &act, NULL);
		write(1, "Press Ctrl+C or wait 10 secs\n", 30);
		sleep(10);

		printf("PID: %d\n", getpid());
		printf("PPID: %d\n", getppid());
	}

	return 0;
}

void handler_function(int sig) {
	printf("Caught signal %d\n", sig);
}
