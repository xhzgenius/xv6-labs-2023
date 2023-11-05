#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

// To get all children of a directory, simply read the fd of this directory. 
// Read the bytes into struct dirent. 

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), '\0', DIRSIZ-strlen(p));
  return buf;
}

void search(char *path, char *pattern)
{
    int fd = open(path, O_RDONLY);
    if(fd<0)
    {
        printf("Cannot open %s\n", path);
        close(fd);
        // exit(0);
    }
    struct stat status;
    fstat(fd, &status);
    if(status.type==T_FILE || status.type==T_DEVICE)
    {
        char *short_name = fmtname(path);
        // printf("Short name of file: %s\n", short_name);
        // printf("Pattern: %s\n", pattern);
        // printf("%d\n", strcmp(short_name, pattern));
        if(!strcmp(short_name, pattern)) printf("%s\n", path);
    }
    else
    {
        char buf[256], *p;
        struct dirent entry;
        while(read(fd, &entry, sizeof(entry))==sizeof(entry))
        {
            if(entry.inum==0) continue;
            char *filename = entry.name;
            // printf("Short name: %s\n", filename);
            if(strcmp(filename, ".")==0 || strcmp(filename, "..")==0) continue;
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
                printf("ls: path too long\n");
                break;
            }
            strcpy(buf, path);
            p = buf+strlen(buf);
            *p++ = '/';
            strcpy(p, filename);
            search(buf, pattern);
        }
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    if(argc!=3)
    {
        printf("Usage: find [dirname] [pattern]\n");
        exit(0);
    }

    char *path = argv[1], *pattern = argv[2];
    search(path, pattern);
    exit(0);
}
