/*
Εργαστήριο ΛΣ 2 / Άσκηση 2 / Ερώτημα 3a / 2025-26
Ονοματεπώνυμο: Στυλιανός Σπανός
AM: 24390042
Τρόπος Μεταγλώττισης: gcc LS2-24390042-Δ4Β-3a.c -o server -lpthread -Wall
*/


/*
  Υλοποιούμε έναν server με χρήση UNIX-domain stream sockets (AF_UNIX) για τοπική επικοινωνία (μέσω κοινού αρχείου).
  Για κάθε νέο client που συνδέεται δημιουργείται ένα νέο thread, ώστε να εξυπηρετούνται ταυτόχρονα πολλοί clients χωρίς να μπλοκάρεται ο server.
  Στην αποδοχή (accept) του κάθε client δεσμεύεται δυναμικά μνήμη για τον file descriptor του socket ώστε να μην αλλοιωθεί η τιμή του από επόμενες συνδέσεις πριν την διαβάσει το νήμα.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/unix_socket"
#define BUFSIZE 256


// Η συνάρτηση που θα εκτελεί κάθε νήμα
void *client_handler(void *arg) {
  int client_sock = *(int*)arg;
  free(arg); // Απελευθερώνουμε την μνήμη του client_sock από πριν

  // Επαναληπτική επικοινωνία, όσο διαβάζουμε το n
  int n;
  while (recv(client_sock, &n, sizeof(int), 0) > 0) {
    if (n <= 0) break;

    // Δεσμεύουμε μνήμη για να διαβάσουμε τους n ακεραίους
    int *numbers = (int*)malloc(n * sizeof(int));
    if (!numbers) {
      perror("malloc");
      continue;
    }

    // Διαβάζουμε όλο τον πίνακα
    recv(client_sock, numbers, n * sizeof(int), MSG_WAITALL);

    // Υπολογισμός μέσου όρου
    long long sum = 0;
    for (int i = 0; i < n; ++i) {
      sum += numbers[i];
    }

    float avg = (float)sum / n;
    free(numbers); // Απελευθερώνουμε την μνήμη του πίνακα


    char response[BUFSIZE];
    memset(response, 0, BUFSIZE);

    if (avg < 50) {
      snprintf(response, BUFSIZE, "Average: %.2f | Sequence OK", avg);
    }
    else {
      snprintf(response, BUFSIZE, "Check Failed");
    }

    // Αποστολή απάντησης στον client 
    send(client_sock, response, BUFSIZE, 0);
  }

  close(client_sock);
  printf("[Server]: A client disconnected\n");
  pthread_exit(NULL);
}




int main() {
  int server_sock;
  struct sockaddr_un server_addr, client_addr;
  socklen_t client_len = sizeof(client_addr);

  // Δημιουργία UNIX stream socket 
  server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_sock == -1) {
    perror("socket");
    exit(1);
  }

  
  // Σβήνουμε το αρχείο του socket σε περίπτωση που έμεινε απο προηγούμενο τρέξιμο
  unlink(SOCKET_PATH);


  // Διαμόρφωση της διεὺθυνσης όπως με το sockaddr_in
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sun_family = AF_UNIX;
  strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

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


  printf("[Server]: Started listening on path: %s\n", SOCKET_PATH);

  // Ο ατέρμονας βρόχος που αποδέχεται clients και δημιουργεί threads
  while(1) {
    // Δυναμικη δέσμευση για αποφυγή race condition
    int *client_sock = (int*)malloc(sizeof(int));
    *client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);

    if (*client_sock == -1) {
      perror("accept");
      free(client_sock);
      continue;
    }

    printf("[Server]: New client got accepted.\n");

    // Δημιουργία του νήματος
    pthread_t tid;
    if (pthread_create(&tid, NULL, &client_handler, (void*)client_sock) != 0) {
      perror("pthread_create");
      free(client_sock);
    } 

    // Απόσπαση του νήματος ώστε να ελευθερώσει του πόρους του όταν τερματίσει
    pthread_detach(tid);
  }

  close(server_sock);
  unlink(SOCKET_PATH);
  return 0;
}
