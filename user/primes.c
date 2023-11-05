#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void proceed(int old_pipe_output_fd)
{
    int newly_piped_fds[2];
    pipe(newly_piped_fds); // From 1 to 0
    int first, now;
    if(read(old_pipe_output_fd, &first, sizeof(int))<=0)
    {
        close(old_pipe_output_fd);
        exit(0); // Read over
    }
    printf("prime %d\n", first);

    int pid = fork();

    if(pid!=0) // Parent process
    {
        close(newly_piped_fds[0]);
        while(read(old_pipe_output_fd, &now, sizeof(int))>0)
        {
            // printf("now=%d, first=%d\n", now, first);
            if(now%first!=0) write(newly_piped_fds[1], &now, sizeof(int));
        }
        close(old_pipe_output_fd);
        close(newly_piped_fds[1]);
        // printf("Prime test of %d ends!\n", first);
        // Wait until the child ends
        wait(0);
        exit(0);
    }
    else // Child process
    {
        close(old_pipe_output_fd);
        close(newly_piped_fds[1]);
        proceed(newly_piped_fds[0]);
    }
}

int main(int argc, char *argv[])
{
    int newly_piped_fds[2];
    pipe(newly_piped_fds); // From 1 to 0
    for(int i = 2;i<35;i++)
    {
        int *buf = &i;
        write(newly_piped_fds[1], buf, sizeof(int));
    }
    close(newly_piped_fds[1]);
    proceed(newly_piped_fds[0]);
    exit(0);
}
