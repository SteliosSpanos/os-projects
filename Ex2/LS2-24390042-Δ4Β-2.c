/*
Εργαστήριο ΛΣ 2 / Άσκηση 2 / Ερώτημα 2 / 2025-26
Ονοματεπώνυμο: Στυλιανός Σπανός
AM: 24390042
Τρόπος Μεταγλώττισης: gcc LS2-24390042-Δ4Β-2.c -o main -lpthread -Wall
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>

// Ορίζουμε τις τρεις σημαφόρους
sem_t sem_what;
sem_t sem_wonderful;
sem_t sem_world;

// Ορίζουμε τις συναρτήσεις που αντιστοιχούν σε κάθε νήμα ξεχωριστά
void *printWhatA(void *arg) {
  (void)arg;

  while (1) {
    sem_wait(&sem_what); // Μειώνουμε την σημαφόρου του ίδιου που θα είναι 1
    printf("What A ");
    fflush(stdout);
    sem_post(&sem_wonderful); // Μετά την κρίσιμη περιοχή αυξάνουμε την σημαφόρο του επόμενου για να την κάνει wait
    sleep(1); // Βάζουμε μια μικρή καθυστέρηση απλα για πιο ομοιόμορφο αποτέλεσμα
  }
}

void *printWonderful(void *arg) {
  (void)arg;

  while (1) {
    sem_wait(&sem_wonderful);
    printf("Wonderful ");
    fflush(stdout);
    sem_post(&sem_world);
    sleep(1);
  }
}

void *printWorld(void *arg) {
  (void)arg;

  while (1) {
    sem_wait(&sem_world);
    printf("World! ");
    fflush(stdout);
    sem_post(&sem_what);
    sleep(1);
  }
}


int main() {
  pthread_t t1, t2, t3;

  // Αρχικοποιούμε τις σημαφόρους όπως είπαμε
  sem_init(&sem_what, 0, 1);
  sem_init(&sem_wonderful, 0, 0);
  sem_init(&sem_world, 0, 0);

  // Δημιουργούμε τα τρία νήματα
  if (pthread_create(&t1, NULL, &printWhatA, NULL) != 0) {
    perror("pthread_create t1");
    exit(EXIT_FAILURE);
  }

  if (pthread_create(&t2, NULL, &printWonderful, NULL) != 0) {
    perror("pthread_create t2");
    exit(EXIT_FAILURE);
  }

  if (pthread_create(&t3, NULL, &printWorld, NULL) != 0) {
    perror("pthread_create t3");
    exit(EXIT_FAILURE);
  }

  // Το κύριο νήμα περιμένει τα υπόλοιπα που σε αυτή τη περίπτωση τρέχουν επ αόριστον
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  pthread_join(t3, NULL);

  // Πρέπει να καταστρέψουμε τις σημαφόρους στο τέλος
  sem_destroy(&sem_what);
  sem_destroy(&sem_wonderful);
  sem_destroy(&sem_world);

  return 0;
}

/*
  Το συγκεκριμένο ερώτημα είναι παρόμοιο με το `ΤΙΚ-ΤΑΚ` πρόβλημα που έχουμε διδαχθεί στη θεωρία.
  Έχουμε 3 σημαφόρους, μία για κάθε thread, και τις αρχικοποιούμε αναλόγως με την σειρά που θέλουμε να επιτύχουμε.
  Την σημαφόρο του νήματος που θέλουμε να ξεκινήσει πρώτο (αυτό που εκτυπώνει "What A") την αρχικοποιούμε σε 1 και τις άλλες σε 0.
  Χρησιμοποιούμε τα wait και post όπως παρακάτω για να γίνεται print το μοτίβο.

  Με την χρήση σημαφόρων ο κώδικας παραμένει πιο απλός και το ίδιο λειτουργικός από το αν χρησιμοποιούσαμε μεταβλητές συνθήκης.
  Στην περίπτωση των condition variables θα έπρεπε επιπλέον να ορίσουμε ένα mutex και μία κοινόχρηστη μεταβλητή την οποία θα έπρεπε να ενημερώνουμε μεσα σε κάθε νήμα.
  Αυτο θα έκανε την υλοποίηση πιο πολύπλοκη χωρίς να χρειάζεται.
*/

