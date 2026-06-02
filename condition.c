#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_OF_CALLS 3

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fire_cond = PTHREAD_COND_INITIALIZER;
int fire_detected = 0;

void *firefighter_task(void *arg) {
  int id = *(int*)arg;
  free(arg);

  printf("[Firefighter %d]: Arrived at the scene and is extinguishing flames...\n", id);
  pthread_exit(NULL);
}

void *fire_department_handler(void *arg) {
  (void) arg;

  pthread_mutex_lock(&lock);
  while (fire_detected == 0) {
    printf("[System]: Monitoring sensors... No fire detected yet.\n");
    pthread_cond_wait(&fire_cond, &lock);
  }

  printf("[Fire Dept]: !!! ALARM RECEIVED !!! Calling department now.\n");

  for (int i = 1; i <= NUM_OF_CALLS; i++) {
    pthread_t fighter;

    int *id = (int*)malloc(sizeof(int));
    if (!id) {
      perror("malloc");
      continue;
    }
    *id = i;

    if (pthread_create(&fighter, NULL, firefighter_task, id) != 0) {
      perror("pthread_create firefighter_task");
      free(id);
      continue;
    }

    pthread_detach(fighter);
  }

  pthread_mutex_unlock(&lock);
  pthread_exit(NULL);
}


void *sensor_monitor(void *arg) {
  (void) arg;
  
  sleep(3);

  pthread_mutex_lock(&lock);
  printf("[Sensor]: Smoke detected! Sending signal...\n");
  fire_detected = 1;
  pthread_cond_signal(&fire_cond); // pthread_cond_broadcast(&fire_cond);
  pthread_mutex_unlock(&lock);

  pthread_exit(NULL);
}


int main() {
  pthread_t dept_thread, sensor_thread;

  if (pthread_create(&dept_thread, NULL, fire_department_handler, NULL) != 0) {
    perror("pthread_create dept_thread");
    exit(EXIT_FAILURE);
  }

  if (pthread_create(&sensor_thread, NULL, sensor_monitor, NULL) != 0) {
    perror("pthread_create sensor_thread");
    exit(EXIT_FAILURE);
  }

  pthread_join(dept_thread, NULL);
  pthread_join(sensor_thread, NULL);

  printf("[Main]: Emergency handled. System shutting down...\n");
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&fire_cond);

  return 0;
}
