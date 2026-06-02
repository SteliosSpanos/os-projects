/*
Εργαστήριο ΛΣ 2 / Άσκηση 1 / Ερώτημα 2 / 2025-26
Ονοματεπώνυμο: Στυλιανός Σπανός
ΑΜ: 24390042
Τρόπος μεταγλώττισης: gcc LS2-24390042-Δ4Β-2.c -o main 
*/



/*

Με βάση τον τρόπο που έχει γραφτεί ο κώδικας, δεν είναι δυνατό να δημιουργηθούν zombie ή orphan διεργασίες στο σύστημα , διότι έχει γίνει διαχείριση του τερματισμού τους μέσω των κατάλληλων εντολών 'wait' σε κάθε επίπεδο της ιεραρχίας.

1. Μια διεργασία γίνεται zombie όταν τερματίζει, αλλά ο γονέας της δεν έχει κάνει wait για να διαβάσει το exit status της, ώστε το ΛΣ να την διαγράψει από τον πίνακα διεργασιών. Στο πρόγρμμα αυτό δεν συμβαίνει γιατί:
- Η Ρ3 δεν γίνεται zombie διότι η Ρ1 καλεί την 'waitpid(pid3, NULL, 0)'.
- Τα παιδιά της Ρ2 (Ρ4, Ρ5, Ρ6, ...) δεν γίνονται zombie διότι η Ρ2 καλεί την 'wait(NULL)' σε ένα loop, μέχρι να τερματίσουν όλα.
- Οι Ρ1 και Ρ2 δεν γίνονται zombie διότι η αρχική Ρ0  καλεί ξεχωριστά 'waitpid' για την καθεμία πριν τερματίσει και εκτελέσει την 'exec'.

2. Μια διεργασία γίνεται orphan όταν ο γονέας της τερματίσει πριν από εκείνη. Στο πρόγραμμα έχουμε εξασφαλίσει ότι κάθε γονέας περιμένει υποχρεωτικά μέχρι να τερματίσουν όλα του τα παιδιά πριν ο ίδιος καλέσει 'exit(0)'. Η πιθανότερη περίπτωση να έχουμε orphans θα ήταν αν περιμέναμε μόνο δύο από τα N παιδία της Ρ2 να τερματίσουν και να εκτυπώσουμε το PID τους (όπως λέει η εκφώνηση).

*/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

#define BUFSIZE 256

int main() {
	/* 
	  Βρισκόμαστε στη διεργασία P0.
	  Ακολουθούμε το παρακάτω σχήμα για την ιεραρχία των δεργασίων:
	                      
	                            P0
                              / \
                            P1   P2
                           /   / |  \
                          P3  P4 P5 P6
	*/ 
	int num_of_childs;
	do {
		printf("Enter the number of childs for P2: ");
		scanf("%d", &num_of_childs);
	} while (num_of_childs <= 0);

	const char msg[] = "hello from your child\n";


	pid_t pid1, pid2;

	// Η διεργασία P1 είναι το πρώτο παιδί της P0
	pid1 = fork();
	if (pid1 < 0) {
		perror("fork failed");
		exit(1);
	}
	else if (pid1 == 0) {
		pid_t pid3;
		char buffer[BUFSIZE];
		int n, pipefd[2]; // Θα χρησιμοποιήσουμε σωλήνωση για την επικοινωνία των διεργασιών

		if (pipe(pipefd) < 0) {
			perror("pipe failed");
			exit(1);
		}

		// Η διεργασία P3 είναι το μοναδικό παιδί της P1
		pid3 = fork();
		if (pid3 < 0) {
			perror("fork failed");
			exit(1);
		}
		else if (pid3 == 0) {
			close(pipefd[0]); // Κλείνουμε το άκρο ανάγνωσης
			write(pipefd[1], msg, sizeof(msg)); // Στέλνουμε το μήνυμα μέσω του σωλήνα
			close(pipefd[1]); // Κλείνουμε το άκρο εγγραφής

			printf("[P3]: PID: %d | PPID: %d\n", getpid(), getppid());
			exit(0);
		}

		close(pipefd[1]); // Κλείνουμε το άκρο εγγραφής
		n = read(pipefd[0], buffer, sizeof(buffer)); // Διαβάζουμε και αποθηκεύουμε στο buffer το μήνυμα
		write(STDOUT_FILENO, buffer, n); // Εμφανίζουμε το μήνυμα στην οθόνη
		close(pipefd[0]); // Κλείνουμε το άκρο ανάγνωσης

		waitpid(pid3, NULL, 0);
		printf("[P1]: PID: %d | PPID: %d\n", getpid(), getppid());
		exit(0);
	}


	// Η διεργασία P2 είναι το δεύτερο παιδί της P0
	pid2 = fork();
	if (pid2 < 0) {
		perror("fork failed");
		exit(1);
	}
	else if (pid2 == 0) {
		// Η P2 έχει Ν παιδία, όπου ο αριθμός δίνεται από τον χρήστη
		for (int i = 0; i < num_of_childs; i++) {
			pid_t pid;

			pid = fork();
			if (pid < 0) {
				perror("fork failed");
				exit(1);
			}
			else if (pid == 0) {
				printf("[P%d]: PID: %d | PPID: %d\n", i+4, getpid(), getppid());
				exit(0);
			}

		}

    // Η Ρ2 περιμένει όλα τα παιδιά της για να μην γίνουν orphans,
    // αλλά εκτυπώνει το PID μόνο των δύο πρώτων που θα τερματίσουν 
		pid_t finished;
    int count = 0;
		while ((finished = wait(NULL)) > 0) {
      count++;

      if (count <= 2)			
        printf("[P2]: Child (PID %d) completed!\n", finished);
    }



		printf("[P2]: PID: %d | PPID: %d\n", getpid(), getppid());
		exit(0);
	}

	int status;
	waitpid(pid1, &status, 0);
	waitpid(pid2, NULL, 0);

	printf("[P0]: PID: %d | PPID: %d\n", getpid(), getppid());

	// Η P0 εκτυπώνει το exit status της P1 αφότου έχει τερματιστεί
	if (WIFEXITED(status)) {
		printf("[P0]: P1 Exit Status: %d\n", WEXITSTATUS(status));
	}

	printf("\n");
	
	// Το κυρίως πρόγραμμα αντικαθίσταται από το 'cat' και εμφανίζει τον κώδικα του αρχείου
	char *args[] = {(char*)"/bin/cat", (char*)__FILE__, NULL};
	if (execv(args[0], args) < 0) {
		perror("exec failed");
		return 1;
	}


	return 0;
}

