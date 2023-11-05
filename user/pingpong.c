#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int piped_fds[2][2];
    pipe(piped_fds[0]); // From piped_fds[0][1] to piped_fds[0][0]
    pipe(piped_fds[1]); // From piped_fds[1][1] to piped_fds[1][0]
    int pid = fork();
    if(pid==0) // Child process
    {
        char read_buf[256];
        int n_read = read(piped_fds[0][0], read_buf, sizeof(read_buf));
        if(n_read>0)
        {
            printf("%d: received ping\n", getpid());
        }
        int n_written = write(piped_fds[1][1], read_buf, n_read);
        if(n_written<0)
        {
            printf("Child write error!\n");
            exit(1);
        }
    }
    else // Parent process
    {
        char write_buf[] = "114514";
        char read_buf[256];
        int n_written = write(piped_fds[0][1], write_buf, sizeof(write_buf));
        if(n_written<0)
        {
            printf("Parent write error!\n");
            exit(1);
        }
        int n_read = read(piped_fds[1][0], read_buf, sizeof(read_buf));
        if(n_read>0)
        {
            printf("%d: received pong\n", getpid());
        }
    }
    exit(0);
}
