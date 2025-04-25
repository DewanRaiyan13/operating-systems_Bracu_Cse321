#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {

    pid_t child_pid, gc1_pid, gc2_pid, gc3_pid;

    printf("1. Parent process ID : %d\n", getpid());
    child_pid = fork();
    
    if (child_pid == 0) {

        printf("2. Child process ID : %d\n", getpid());

        gc1_pid = fork();
        if (gc1_pid == 0) {
            printf("3. Grandchild process ID : %d\n", getpid());
            exit(EXIT_SUCCESS);
        }

        gc2_pid = fork();
        if (gc2_pid == 0) {
            printf("4. Grandchild process ID : %d\n", getpid());
            exit(EXIT_SUCCESS);
        }

        gc3_pid = fork();
        if (gc3_pid == 0) {
            printf("5. Grandchild process ID : %d\n", getpid());
            exit(EXIT_SUCCESS);
        }

        exit(EXIT_SUCCESS);
    } 

    wait(NULL);  
    return 0;
}

