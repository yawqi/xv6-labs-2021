#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int nthread = 1;
static int round = 0;

struct barrier {
  pthread_mutex_t barrier_mutex;
  pthread_cond_t barrier_cond;
  volatile int
      arrive_thread; // Number of threads that have reached this round of the
  volatile int
      leave_thread; // Number of threads that have reached this round of the
  int round;        // Barrier round
} bstate;

static void barrier_init(void) {
  assert(pthread_mutex_init(&bstate.barrier_mutex, NULL) == 0);
  assert(pthread_cond_init(&bstate.barrier_cond, NULL) == 0);
  bstate.arrive_thread = 0;
  bstate.leave_thread = 0;
}

static void barrier() {
  // YOUR CODE HERE
  //
  // Block until all threads have called barrier() and
  // then increment bstate.round.

  while (bstate.leave_thread != 0) {
  }
  pthread_mutex_lock(&bstate.barrier_mutex);
  bstate.arrive_thread += 1;
  if (bstate.arrive_thread != nthread) {
    while (bstate.arrive_thread != 0) {
      pthread_cond_wait(&bstate.barrier_cond, &bstate.barrier_mutex);
    }
    bstate.leave_thread += 1;
    pthread_cond_broadcast(&bstate.barrier_cond);
  } else {
    bstate.round += 1;
    bstate.leave_thread = 1;
    bstate.arrive_thread = 0;
    pthread_cond_broadcast(&bstate.barrier_cond);
    pthread_mutex_unlock(&bstate.barrier_mutex);
    while (bstate.leave_thread != nthread) {
    }
    pthread_mutex_lock(&bstate.barrier_mutex);
    bstate.leave_thread = 0;
  }
  pthread_mutex_unlock(&bstate.barrier_mutex);
}

static void *thread(void *xa) {
  long n = (long)xa;
  long delay;
  int i;

  for (i = 0; i < 20000; i++) {
    int t = bstate.round;
    assert(i == t);
    barrier();
    usleep(random() % 100);
  }

  return 0;
}

int main(int argc, char *argv[]) {
  pthread_t *tha;
  void *value;
  long i;
  double t1, t0;

  if (argc < 2) {
    fprintf(stderr, "%s: %s nthread\n", argv[0], argv[0]);
    exit(-1);
  }
  nthread = atoi(argv[1]);
  tha = malloc(sizeof(pthread_t) * nthread);
  srandom(0);

  barrier_init();

  for (i = 0; i < nthread; i++) {
    assert(pthread_create(&tha[i], NULL, thread, (void *)i) == 0);
  }
  for (i = 0; i < nthread; i++) {
    assert(pthread_join(tha[i], &value) == 0);
  }
  printf("OK; passed\n");
}
