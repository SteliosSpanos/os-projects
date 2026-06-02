/*
Εργαστήριο ΛΣ 2 / Άσκηση 2 / Ερώτημα 3b / 2025-26
Ονοματεπώνυμο: Στυλιανός Σπανός
AM: 24390042
Τρόπος Μεταγλώττισης: gcc LS2-24390042-Δ4Β-3b.c -o client -lpthread -Wall
*/


/*
  Υλοποιούμε έναν client που επικοινωνεί με τον τοπικό server μέσω ενός UNIX-domain stream socket.
  Ο client ζητά συνεχώς από τον χρήστη να εισάγει νέες ακολουθίες αριθμών, μέχρι ο χρήστης να δώσει την τιμή 0 για το μέγεθος n.
  Γίνεται έλεγχος εισόδου και σε περίπτωση λάθους καθαρίζεται ο buffer ώστε να μην κρασάρει το πρόγραμμα ή να σταλθούν σκουπίδια.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/unix_socket"
#define BUFSIZE 256

int main() {
  int sock_fd;
  struct sockaddr_un server_addr;

  // Δημιουργία UNIX stream socket
  sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    perror("socket");
    exit(1);
  }

  // Ρύθμιση της διεύθυνσης όπως με το sockaddr_in
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sun_family = AF_UNIX;
  strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);


  // Κάνουμε connect στον server 
  if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("connect");
    close(sock_fd);
    exit(1);
  }

  printf("Successfully connected to server.\n\n");


  // Επαναληπτική επικοινωνία μέχρι να δώσουμε 0 στη είσοδο
  while (1) {
    int n;
    printf("How many numbers (n) is the sequence gonna have (0 for exit): ");

    // Έλεγχος για το n 
    if (scanf("%d", &n) != 1) {
      printf("[Error]: Please enter a valid integer number.\n\n");

      // Αδιεάζουμε τον buffer του πληκτρολογίου μέχρι το Enter
      int ch;
      while((ch = getchar()) != '\n' && ch != EOF);
      continue;
    }

    if (n <= 0) {
      printf("Terminating client...\n");
      break;
    }


    // Δεσμεύουμε μνήμη για την ακολουθία που θα στείλουμε
    int *numbers = (int*)malloc(n * sizeof(int));
    if (!numbers) {
      perror("malloc");
      continue;
    }

    printf("Enter %d integer numbers:\n", n);

    // Έλεγχος για τον πίνακα
    int input_valid = 1;
    for (int i = 0; i < n; ++i) {
      if (scanf("%d", &numbers[i]) != 1) {
        printf("[Error]: Non-integer value found. The sequence is canceled.\n\n");

        // Αδειάζουμε τον buffer
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        input_valid = 0;
        break;
      }
    }

    // Αν η είσοδος ήταν λάθος, απελευθερώνουμε την μνήμη και ξεκινάμε από την αρχή
    if (!input_valid) {
      free(numbers);
      continue;
    }


    // Στέλνουμε το n στον server
    send(sock_fd, &n, sizeof(int), 0);

    // Στέλνουμε όλο τον πίνακα στον server 
    send(sock_fd, numbers, n * sizeof(int), 0);

    // Απελευθερώνουμε την μνήμη του πίνακα που δεν την χρειαζόμαστε
    free(numbers);

    // Λήψη της απάντησης
    char response[BUFSIZE];
    memset(response, 0, BUFSIZE);
    recv(sock_fd, response, BUFSIZE, 0);

    printf("=> [Server Reply]: %s\n", response);
  }

  close(sock_fd);
  return 0;
}
