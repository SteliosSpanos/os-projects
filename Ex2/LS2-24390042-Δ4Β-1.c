/*
Εργαστήριο ΛΣ 2 / Άσκηση 2 / Ερώτημα 1 / 2025-26
Ονοματεπώνυμο: Στυλιανός Σπανός
AM: 24390042
Τρόπος Μεταγλώττισης: gcc LS2-24390042-Δ4Β-1.c -o main -lpthread -Wall
*/

/*
  Το πρόγραμμα υπολογίζει το εσωτερικό γινόμενο δύο διανυσμάτων μεγέθους n, χρησιμοποιώντας p νήματα.
  Το n πρέπει να είναι ακέραιο πολλαπλάσιο του p, ώστε ο φόρτος εργασίας των νημάτων να μοιράζεται ισάξια.
  Η κρίσιμη περιοχή βρίσκεται στο τέλος κάθε εκτέλεσης ενός νήματος (total_sum) και προστατεύεται από μία σημαφόρο.
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

// Global μεταβλητές για να τις βλέπουν τα threads
sem_t sem;
int *a, *b;
int n, p; // Το n είναι ακέραιο πολλαπλάσιο του p
long long total_sum = 0; // long long για να μην έχουμε υπερχείλιση


// Συνάρτηση που εκτελεί κάθε νήμα
void *calculateSum(void *arg) {
  int tid = *(int *)arg; // Μετατρέπουμε την void* παράμετρο σε int

  // Μοιράζουμε τον αριθμό των επιμέρους γινομένων σύμφωνα με τον αριθμό των νημάτων
  int num_prods = n/p;
  int start_idx = tid * num_prods;
  int end_idx = start_idx + num_prods;

  long long local_sum = 0; // Ορίζουμε το τοπικό άθροισμα που στο τέλος προσθέτουμε στο συνολικό

  for (int i = start_idx; i < end_idx; ++i) {
    local_sum += (long long)a[i] * b[i];
  }

  // Κρίσιμη Περιοχή: Προσθέτουμε στο total_sum
  sem_wait(&sem);
  total_sum += local_sum;
  sem_post(&sem);

  pthread_exit(NULL);
}



int main() {
  sem_init(&sem, 0, 1); // Αρχικοποιούμε την σημαφόρο σε 1

  // Εκτυπώνουμε τα μηνύματα για την εισαγωγή τιμών
  // Ο βρόχος συνεχίζει αν το n δεν είναι ακέραιο πολλαπλάσιο του p
  do {
    printf("---- MUST: n > 0 | p > 0 | n %% p == 0 ----\n");

    printf("Enter the size of the two vectors (n): ");
    scanf("%d", &n);

    printf("Enter the number of threads (p): ");
    scanf("%d", &p);
  } while (n < 1 || p < 1 || p > n || n % p != 0);



  // Δεσμεύουμε μνήμη για τα δύο διανύσματα με μέγεθος n
  a = (int*)malloc(n * sizeof(int));
  b = (int*)malloc(n * sizeof(int));
  if (!a || !b) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  // Γεμίζουμε τους πίνακες με τυχαίους αριθμούς π.χ μέχρι το 100
  srand(time(NULL));
  for (int i = 0; i < n; ++i) {
    a[i] = rand() % 100;
    b[i] = rand() % 100;
  }


  // Δεσμεύουμε μνήμη για του πίνακες των νημάτων για την αποφυγή stack overflow σε περίπτωση μεγάλου p
  int *thread_ids = (int*)malloc(p * sizeof(int));
  pthread_t *tids = (pthread_t*)malloc(p * sizeof(pthread_t));
  if (!thread_ids || !tids) {
    perror("malloc threads");
    exit(EXIT_FAILURE);
  }

  // Ξεκινάμε την χρονομέτρηση πριν την δημιουργία των νημάτων
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);


  // Δημιουργούμε τα p νήματα
  for (int i = 0; i < p; ++i) {
    thread_ids[i] = i;
    if (pthread_create(&tids[i], NULL, &calculateSum, &thread_ids[i]) != 0) {
      perror("pthread_create");
      exit(EXIT_FAILURE);
    }
  }


  // Το κύριο νήμα περιμένει τα υπόλοιπα νήματα να τερματίσουν
  for (int i = 0; i < p; ++i) {
    pthread_join(tids[i], NULL);
  }


  // Σταματάμε την χρονομέτρηση αφού τερματίσουν όλα τα νήματα και βρίσκουμε τον συνολικό χρόνο σε sec
  clock_gettime(CLOCK_MONOTONIC, &end);
  double exec_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;


  printf("Total Sum: %lld\n", total_sum);
  printf("Execution Time with %d threads: %f sec\n", p, exec_time);


  free(a);
  free(b);
  free(thread_ids);
  free(tids);
  sem_destroy(&sem); // Καταστρέφουγε την σημαφόρο

  return 0;
}


/*
  Το πρόγραμμα υπολογίζει το εσωτερικό γινόμενο δύο διανυσμάτων μεγέθους n, χρησιμοποιώντας p νήματα.
  Το n πρέπει να είναι ακέραιο πολλαπλάσιο του p, ώστε ο φόρτος εργασίας των νημάτων να μοιράζεται ισάξια.
  Η κρίσιμη περιοχή βρίσκεται στο τέλος κάθε εκτέλεσης ενός νήματος (total_sum) και προστατεύεται από μία σημαφόρο.
  Για πολύ μεγάλο μέγεθος διανύσματος n = 100,000,000 πήραμε τις παρακάτω χρονομετρήσεις για του αντίστοιχους αριθμοὺς νημάτων:

  - p = 1: ~ 0.287 sec
  - p = 2: ~ 0.152 sec
  - p = 4: ~ 0.145 sec
  - p = 8: ~ 0.141 sec

  Καθώς αυξάνεται ο αρθμός νημάτων, ο χρόνος εκτέλεσης μειώνεται οπώς ήταν αναμενόμενο, αλλά η μείωση δεν είναι γραμμική λόγω του overhead της δημιουργίας των νημάτων και των καθυστερήσεων και συγχρονισμού στην μνήμη.
*/
