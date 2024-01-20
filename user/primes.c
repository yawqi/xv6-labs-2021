#include "kernel/types.h"
#include "user/user.h"

void helper(int prev_read_pipe) {
  int p = 0;
  int next_pipe[2] = {0};
  int child = 0;
  int value = 0;
  if (!read(prev_read_pipe, &p, sizeof(int))) {
    return;
  }
  printf("prime %d\n", p);
  pipe(next_pipe);
  child = fork();
  if (child) {
    close(next_pipe[0]);
    while (read(prev_read_pipe, &value, sizeof(int))) {
      if (value % p != 0)
        write(next_pipe[1], &value, sizeof(int));
    }
    close(next_pipe[1]);
  } else {
    close(next_pipe[1]);
    helper(next_pipe[0]);
    close(next_pipe[0]);
  }
  wait(0);
}

int main() {
  int initial_pipe[2];
  int child;
  int start = 2;
  int end = 35;
  int i = 0;
  pipe(initial_pipe);
  child = fork();
  if (child) {
    close(initial_pipe[0]);
    for (i = start; i <= end; i++) {
      write(initial_pipe[1], &i, sizeof(int));
    }
    close(initial_pipe[1]);
  } else {
    close(initial_pipe[1]);
    helper(initial_pipe[0]);
    close(initial_pipe[0]);
    exit(0);
  }

  wait(0);
  exit(0);
}
