#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

#define BLOCKSIZE 512

void xargs(char* line, int argc, char **argv) {
    if (*line == 0) return; // skip empty line
    int arg_beg = 0;
    int arg_end = 0;
    int reach_end = 0;
    int arg_num = 0;
    char* child_arg[MAXARG+1];
    for(int i = 1; i < argc; ++i) {
        child_arg[arg_num++] = argv[i];
    }
    while(reach_end == 0 && arg_num < MAXARG) {
        while(*(line + arg_end) != 0 && *(line + arg_end) != ' ') arg_end++;
        if (*(line + arg_end) == 0) {
            reach_end = 1; // this is the last arg
        }
        child_arg[arg_num++] = line + arg_beg;
        arg_beg = arg_end + 1;
        arg_end = arg_end + 1;
    }
    child_arg[arg_num] = 0;
    if (fork() == 0) {
        exec(argv[1], child_arg);
        fprintf(2, "exec failed\n");
		exit(1);
    } else {
        wait(0);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "Usage: xargs <command> ...\n");
        exit(0);
    }

    char buffer[513];
    // fprintf(1, "[debug]origin arg: %s,%s,%s\n", argv[0], argv[1], argv[2]);

    buffer[0] = 0;
    int buf_beg = 0;
    int buf_end = 0;
    int read_bytes;
    while((read_bytes = read(0, buffer+buf_end, BLOCKSIZE)) != 0) {
        for(int i = 0; i < read_bytes; ++i) {
            if (*(buffer+buf_end+i) == '\n') {
                buffer[buf_end+i] = 0;
                // buffer[buf_beg:buf_beg+i] is one line
                xargs(buffer+buf_beg, argc, argv);
                buf_beg=buf_end+i+1;
            }
        }
        buf_end += read_bytes;
    }
    for(int i = buf_beg; i < buf_end; ++i) {
        if (*(buffer+i) == '\n') {
            buffer[i] = 0;
            // buffer[buf_beg:i] is one line
            xargs(buffer+buf_beg, argc, argv);
            buf_beg=i+1;
        }
    }
    exit(0);
}