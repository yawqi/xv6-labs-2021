#include "kernel/types.h"
#include "user/user.h"

int main() {
  int ping_pipes[2];
  int pong_pipes[2];
  int pid;
  char c = 'a';
  char buf[2] = {0};
  if (pipe(ping_pipes) < 0) {
    printf("create ping pipes failed\n");
    exit(-1);
  }

  if (pipe(pong_pipes) < 0) {
    printf("create pong pipes failed\n");
    exit(-1);
  }

  if (fork()) {
    close(ping_pipes[0]);
    close(pong_pipes[1]);
    pid = getpid();
    write(ping_pipes[1], &c, 1);
    if (read(pong_pipes[0], buf, 1)) {
      printf("%d: received pong\n", pid);
    }
    close(ping_pipes[1]);
    close(pong_pipes[0]);
    exit(0);
  } else {
    close(ping_pipes[1]);
    close(pong_pipes[0]);
    pid = getpid();
    if (read(ping_pipes[0], buf, 1)) {
      printf("%d: received ping\n", pid);
    }
    write(pong_pipes[1], &c, 1);
    close(ping_pipes[0]);
    close(pong_pipes[1]);
    exit(0);
  }
}
