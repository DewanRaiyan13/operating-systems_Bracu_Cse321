#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> 


int main() {
    pid_t child_pid = fork();
    if (child_pid == 0) {
        pid_t grandchild_pid = fork();
        

        if (grandchild_pid == 0) {
            printf("I am grandchild\n");
            exit(EXIT_SUCCESS);
            
            
        } else {
            wait(NULL);
            printf("I am child\n");
            exit(EXIT_SUCCESS);
        }

    } else {
        wait(NULL);
        printf("I am parent\n");
    }

    return 0;
}

