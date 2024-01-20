#include "kernel/types.h"

#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PATH_BUFSIZ 512

int is_dot_or_dotdot(char *name) {
  if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0 ||
      strcmp(name, "./") == 0 || strcmp(name, "../") == 0 ||
      strcmp(name, "") == 0) {
    return 1;
  }
  return 0;
}

void find_helper(char *path, const char *target) {
  int dirfd, len;
  struct dirent ent = {0};
  struct stat st;
  char nxt_path[PATH_BUFSIZ] = {0};
  char *p;
  dirfd = open(path, 0);
  if (dirfd < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  strcpy(nxt_path, path);
  len = strlen(nxt_path);
  if (nxt_path[len - 1] != '/') {
    nxt_path[len] = '/';
    len += 1;
  }

  p = nxt_path + len;
  while (read(dirfd, &ent, sizeof(ent)) == sizeof(ent)) {
    if (ent.inum == 0)
      continue;
    strcpy(p, ent.name);
    len = strlen(p);
    if (is_dot_or_dotdot(p) > 0)
      continue;
    if (strcmp(p, target) == 0) {
      printf("%s\n", nxt_path);
    }
    if (stat(nxt_path, &st) < 0) {
      fprintf(2, "find: cannot stat %s %s %s\n", path, nxt_path, p);
      exit(1);
    }
    if (st.type == T_DIR) {
      find_helper(nxt_path, target);
    }
  }
  close(dirfd);
}

int main(int argc, char *argv[]) {
  char path[PATH_BUFSIZ] = {0};
  char target[DIRSIZ + 1] = {0};
  if (argc != 3) {
    printf("Usage: find DIR FILE\n");
    exit(0);
  }

  if (strlen(argv[1]) > PATH_BUFSIZ - 1) {
    printf("DIR name too long\n");
    exit(1);
  }
  strcpy(path, argv[1]);

  if (strlen(argv[2]) > DIRSIZ) {
    printf("FILE name too long\n");
    exit(1);
  }
  strcpy(target, argv[2]);
  find_helper(path, target);
  exit(0);
}
