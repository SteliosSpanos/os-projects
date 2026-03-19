#include <stdio.h>
#include <pthread.h>

void printString(void *) {
	int i = 0;
	while (i < 20) {
		printf("Hello from thread\n");
	}
}

int main() {
	pthread_t tid;
	pthread_create(&tid, NULL, (void*)&printString, NULL);

	int i = 0;
	while (i < 20) {
		printf("Hello from main\n");
		i++;
	}

	return 0;
}



