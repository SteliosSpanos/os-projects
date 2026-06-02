#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8888
#define BUFSIZE 1024


void *client_handler(void *arg) {
  int client_sock = *(int*)arg;
  free(arg);

  int n;
  while (recv(client_sock, &n, sizeof(int), 0) > 0) {
    if (n <= 0) break;

    int *numbers = (int*)malloc(n * sizeof(int));
    if (!numbers) {
      perror("malloc");
      continue;
    }

    int bytes_received = recv(client_sock, numbers, n * sizeof(int), MSG_WAITALL);
    if (bytes_received <= 0) {
      if (bytes_received == 0) {
        printf("[Server]: Client disconnected unexpectedly before sending data\n");
      }
      else {
        perror("recv");
      }

      free(numbers);
      break;
    }

    long long sum = 0;
    for (int i = 0; i < n; i++) {
      sum += numbers[i];
    }

    float avg = (float) sum / n;
    free(numbers);

    char response[BUFSIZE];
    memset(response, 0, BUFSIZE);

    if (avg < 50) {
      snprintf(response, BUFSIZE, "Average: %.2f | Sequence OK", avg);
    }
    else {
      snprintf(response, BUFSIZE, "Check Failed");
    }

    send(client_sock, response, BUFSIZE, 0);
  }

  close(client_sock);
  printf("[Server]: A client disconnected\n");
  pthread_exit(NULL);
}



int main() {
  int server_sock;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_len = sizeof(client_addr);

  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock == -1) {
    perror("socket");
    exit(1);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  inet_pton(AF_INET, "192.168.2.5", &server_addr.sin_addr); // server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT); // Host to Network Byte Order (Little-Endian to Big-Endian)


  if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("bind");
    close(server_sock);
    exit(1);
  }

  if (listen(server_sock, 10) == -1) {
    perror("listen");
    close(server_sock);
    exit(1);
  }

  printf("[Server]: Starting listening on IP 192.168.2.5 and port 8888...\n");

  while (1) {
    int *client_sock = (int*)malloc(sizeof(int));
    if (!client_sock) {
      perror("malloc");
      continue;
    }

    *client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
    if (*client_sock == -1) {
      perror("accept");
      free(client_sock);
      continue;
    }
    
    printf("[Server]: New client got accepted\n");


    pthread_t tid;
    if (pthread_create(&tid, NULL, &client_handler, (void*)client_sock) != 0) {
      perror("pthread_create");
      free(client_sock);
    }

    pthread_detach(tid);
  } 

  close(server_sock);
  return 0;
}

