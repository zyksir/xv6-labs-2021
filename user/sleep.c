#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){
  if (argc < 2) {
    fprintf(2, "Usage: sleep integer\n");
    exit(1);
  }
  sleep(atoi(argv[1]));
  int ret = sleep(atoi(argv[1]));
  if (ret != 0) {
    fprintf(2, "Fail to sleep %s with return code %d\n", argv[1], ret);
    exit(1);
  }
  exit(0);
}
