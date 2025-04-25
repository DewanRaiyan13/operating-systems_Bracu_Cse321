#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int arg_count, char *arg_values[]) {
    char *arg_list[arg_count + 1]; 

    arg_list[0] = "./sort";  
    for (int i = 1; i < arg_count; i++) {
        arg_list[i] = arg_values[i];
    }
    arg_list[arg_count] = NULL;

    pid_t child_pid = fork();

    if (child_pid == 0) {
        execv("./sort", arg_list);
    } else {
        wait(NULL);

        arg_list[0] = "./oddeven"; 
        execv("./oddeven", arg_list);
    }

    return 0;
}

