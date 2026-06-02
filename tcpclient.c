#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFSIZE 1024

int main() {
  int client_sock;
  struct sockaddr_in server_addr;
  const char *server_ip = "192.168.2.5"; 

  client_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (client_sock == -1) {
    perror("socket");
    exit(1);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
  server_addr.sin_port = htons(PORT);

  if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("connect");
    close(client_sock);
    exit(1);
  }

  printf("Successfully connected to server!\n\n");


  while (1) {
    int n;
    printf("How many numbers (n) is the sequence gonna have (0 for exit): ");

    if (scanf("%d", &n) != 1) {
      printf("Error: Please enter valid integer number\n\n");

      int ch;
      while ((ch = getchar()) != '\n' && ch != EOF);
      continue;
    }

    if (n <= 0) {
      printf("Terminating client...\n");
      break;
    }


    int *numbers = (int*)malloc(n * sizeof(n));
    if (!numbers) {
      perror("malloc");
      continue;
    }

    printf("Enter %d integer numbers:\n", n);

    int input_valid = 1;
    for (int i = 0; i < n; i++) {
      if (scanf("%d", &numbers[i]) != 1) {
        printf("Error: Non-integer value found. The sequence is cancelled\n\n");

        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        input_valid = 0;
        break;
      }
    }

    if (!input_valid) {
      free(numbers);
      continue;
    }


    send(client_sock, &n, sizeof(int), 0);
    send(client_sock, numbers, n * sizeof(int), 0);

    free(numbers);

    char response[BUFSIZE];
    memset(response, 0, BUFSIZE);
    recv(client_sock, response, BUFSIZE, 0);

    printf("==> [Server Reply]: %s\n", response);
  }

  close(client_sock);
  return 0;
}
