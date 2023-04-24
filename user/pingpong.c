#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READ 0
#define WRITE 1

int main(int argc, char *argv[]){
  int p[2];
  int pid;
  pipe(p);
  if (fork() == 0) {
    // child
    char info[2] = "b";
    char c[2];
    if (read(p[READ], c, 1) != 1) {
        fprintf(2, "failed to read in child\n");
		exit(1);
    }
    c[1] = 0;
    close(p[READ]);

    pid = getpid();
    // fprintf(1, "%d: received %s\n", pid, c);
    fprintf(1, "%d: received ping\n", pid);

    if (write(p[WRITE], info, 1) != 1) {
        fprintf(2, "failed to write in child\n");
		exit(1);
    }
    close(p[WRITE]);
  } else {
    char info[2] = "a";
    char c[2];
    pid = getpid();
    if (write(p[WRITE], info, 1) != 1) {
        fprintf(2, "failed to write in parent\n");
		exit(1);
    }
    close(p[WRITE]);
    if (read(p[READ], c, 1) != 1) {
        fprintf(2, "failed to read in parent\n");
		exit(1);
    }
    c[1] = 0;
    close(p[READ]);
    wait(0);
    // fprintf(1, "%d: received %s\n", pid, c);
    fprintf(1, "%d: received pong\n", pid);
  }
  exit(0);
}
