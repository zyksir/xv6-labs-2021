#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READ 0
#define WRITE 1

__attribute__((noreturn))  void childProcess(int p_read) {
    int hasChild = 0;
    char buffer[5];
    int ret = read(p_read, buffer, 4);
    if (ret == 0) { exit(0); }
    int num = *((int*)buffer);
    fprintf(1, "prime %d\n", num);

    int p[2];
    while(ret != 0) {
        ret = read(p_read, buffer, 4);
        if (ret == 0) { break; }
        int n = *((int*)buffer);
        if (n % num == 0) { continue; }
        if (hasChild == 0) {
            hasChild = 1;
            pipe(p);
            if (fork() == 0) {
                close(p[WRITE]);
                childProcess(p[READ]);
            } else {
                close(p[READ]);
                write(p[WRITE], &n, 4);
            }
        } else {
            write(p[WRITE], &n, 4);
        }
    }
    close(p_read);
    if (hasChild == 1) {
        close(p[WRITE]);
        wait(0);
    }
    exit(0);
}

int main(int argc, char *argv[]){
    int p[2];
    pipe(p);
    if (fork() == 0) {
        close(p[WRITE]);
        childProcess(p[READ]);
    } else {
        close(p[READ]);
        for(int num = 2; num < 35; num++) {
            write(p[WRITE], &num, 4);
        } 
        close(p[WRITE]);
    }
    wait(0);
    exit(0);
}
