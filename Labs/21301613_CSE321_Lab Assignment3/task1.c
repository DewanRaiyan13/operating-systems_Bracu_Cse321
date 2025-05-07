#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

struct shared {
    char select[100];
    int balance;
};

int main() {
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(1);
    }

    int shm_id = shmget(IPC_PRIVATE, sizeof(struct shared), 0666 | IPC_CREAT);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }

    struct shared *shm_data = (struct shared *)shmat(shm_id, NULL, 0);
    if (shm_data == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    printf("Provide Your Input From Given Options:\n");
    printf("1. Type a to Add Money\n");
    printf("2. Type w to Withdraw Money\n");
    printf("3. Type c to Check Balance\n");
    char user_input[100];
    fgets(user_input, sizeof(user_input), stdin);
    user_input[strcspn(user_input, "\n")] = 0;

    strncpy(shm_data->select, user_input, sizeof(shm_data->select));
    shm_data->balance = 1000;

    printf("Your selection: %s\n\n", shm_data->select);

    pid_t pid = fork();
    if (pid == 0) {
        close(pipe_fd[0]);
        char pipe_msg[200] = "Thank you for using\n";
        if (strcmp(shm_data->select, "a") == 0) {
            printf("Enter amount to be added:\n");
            int value;
            scanf("%d", &value);
            if (value > 0) {
                shm_data->balance += value;
                printf("Balance added successfully\n");
                printf("Updated balance after addition:\n%d\n", shm_data->balance);
            } else {
                printf("Adding failed, Invalid amount\n");
            }
        } else if (strcmp(shm_data->select, "w") == 0) {
            printf("Enter amount to be withdrawn:\n");
            int value;
            scanf("%d", &value);
            if (value > 0 && value < shm_data->balance) {
                shm_data->balance -= value;
                printf("Balance withdrawn successfully\n");
                printf("Updated balance after withdrawal:\n%d\n", shm_data->balance);
            } else {
                printf("Withdrawal failed, Invalid amount\n");
            }
        } else if (strcmp(shm_data->select, "c") == 0) {
            printf("Your current balance is:\n%d\n", shm_data->balance);
        } else {
            printf("Invalid selection\n");
        }
        write(pipe_fd[1], pipe_msg, strlen(pipe_msg));
        close(pipe_fd[1]);
        shmdt(shm_data);
        exit(0);
    } else {
        close(pipe_fd[1]);
        wait(NULL);
        char pipe_msg[200] = {0};
        read(pipe_fd[0], pipe_msg, sizeof(pipe_msg));
        printf("%s", pipe_msg);
        close(pipe_fd[0]);
        shmdt(shm_data);
        shmctl(shm_id, IPC_RMID, NULL);
    }
    return 0;
}
