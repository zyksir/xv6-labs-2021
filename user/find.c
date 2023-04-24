#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

static const char current_dir[2] = ".";
static const char parent_dir[3] = "..";

void find(char* path, const char* target_file_name) {
    int fd;
    char buf[512], *p;
    struct dirent de;
    struct stat st;

    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
      printf("ls: path too long\n");
      return;
    }

    if((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if (st.type != T_DIR) {
        fprintf(2, "find: %s is not a directory\n", path);
        close(fd);
        return;
    }

    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0) continue;
        if (strcmp(de.name, current_dir) == 0 || strcmp(de.name, parent_dir) == 0) continue;
        memmove(p, de.name, DIRSIZ);
        if(stat(buf, &st) < 0){
            printf("ls: cannot stat %s\n", buf);
            continue;
        }
        if (st.type == T_FILE) {
            if (strcmp(target_file_name, de.name) == 0) {
                printf("%s\n", buf);
            }
        } else if (st.type == T_DIR) {
            find(buf, target_file_name);
        }
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(2, "Usage: sleep <dir name> <file name>\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}
