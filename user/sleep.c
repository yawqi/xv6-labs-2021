#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  int secs;
  if (argc != 2) {
    printf("usage: sleep #");
    exit(0);
  }

  secs = atoi(argv[1]);
  sleep(secs);
  exit(0);
}
