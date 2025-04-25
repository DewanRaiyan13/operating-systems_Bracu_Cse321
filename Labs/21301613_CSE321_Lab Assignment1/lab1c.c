#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

int main()
{
    int sync[2];
    if (pipe(sync) != 0) printf("Pipe error opening\n");

    pid_t pid1, pid2, pid3;
    int count = 0;
    pid_t main_pid = getpid();

    write(sync[1], &count, sizeof(int));
    pid1 = fork();
    pid2 = fork();
    pid3 = fork();
    pid_t curr_pid = getpid();
    printf("Parent: %d, Child: %d\n", getppid(), curr_pid);
    if (curr_pid != main_pid && curr_pid % 2 != 0) {
        fork();
    }

    read(sync[0], &count, sizeof(int));
    count++;
    write(sync[1], &count, sizeof(int));
    close(sync[1]);

    wait(NULL);
    wait(NULL);
    wait(NULL);

    if (getpid() == main_pid) {
        read(sync[0], &count, sizeof(int));
        close(sync[0]);
        printf("Total: %d\n", count);
    }

    return 0;
}

