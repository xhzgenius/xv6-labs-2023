#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"


int main(int argc, char *argv[MAXARG])
{
    char *new_args[MAXARG];
    int new_argc = 0;
    char now;
    uint idx = 0;
    memset(new_args, 0, sizeof(new_args));
    while(read(0, &now, 1)==1) // Read 1 char
    {
        if(now==' ')
        {
            idx = 0;
        }
        else if(now=='\n')
        {
            idx = 0;
            // Execute
            char *final_args[MAXARG];
            memset(final_args, 0, sizeof(final_args));
            int final_argc = argc+new_argc;
            // printf("Final argc: %d\n", final_argc);
            if(final_argc>MAXARG)
            {
                printf("Too many args! (Max %d)\n", MAXARG);
                continue;
            }
            for(int i = 0;i<argc;i++) final_args[i] = argv[i];
            for(int i = 0;i<new_argc;i++) final_args[argc+i] = new_args[i];
            // for(int i = 0;final_args[i];i++) printf("arg %d: %s ", i, final_args[i]);
            int pid = fork();
            if(pid==0) // Child
            {
                exec(final_args[1], final_args+1);
            }
            wait(0);
            // Clear
            idx = 0;
            new_argc = 0;
            memset(new_args, 0, sizeof(new_args));
        }
        else
        {
            if(idx==0)
            {
                new_args[new_argc] = malloc(256);
                new_argc++;
            }
            new_args[new_argc-1][idx] = now;
            idx++;
        }
    }
    exit(0);
}
