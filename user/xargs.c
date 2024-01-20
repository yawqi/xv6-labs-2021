#include "kernel/types.h"

#include "kernel/param.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  char *args[MAXARG] = {0};
  char buf[512] = {0};
  char c;
  int i;
  int last = 0;
  if (argc < 2) {
    printf("Usage: xargs EXEC [args]\n");
    exit(0);
  }

  for (i = 1; i < argc; i++) {
    args[i - 1] = argv[i];
  }

  i = 0;
  while (read(0, &c, 1) == 1) {
    if (c == '\n') {
      args[argc - 1] = buf;
      if (fork() == 0) {
        exec(argv[1], args);
      }
      wait(0);
      memset(buf, 0, sizeof(buf));
      i = 0;
      last = 1;
    } else {
      buf[i++] = c;
      last = 0;
    }
  }

  if (last == 0) {
    args[argc - 1] = buf;
    if (fork() == 0) {
      exec(argv[1], args);
    }
    wait(0);
  }
  exit(0);
}
