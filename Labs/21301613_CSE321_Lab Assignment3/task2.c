#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

struct msg {
    long int type;
    char txt[6];
};

int main() {
    key_t mqueue_key = 3210;
    int msg_id = msgget(mqueue_key, 0666 | IPC_CREAT);
    if (msg_id == -1) {
        perror("msgget");
        exit(1);
    }

    char user_workspace[100];
    printf("Please enter the workspace name:\n");
    fgets(user_workspace, sizeof(user_workspace), stdin);
    user_workspace[strcspn(user_workspace, "\n")] = 0;

    if (strcmp(user_workspace, "cse321") != 0) {
        printf("Invalid workspace name\n");
        msgctl(msg_id, IPC_RMID, NULL);
        return 0;
    }

    struct msg gen_otp_req;
    gen_otp_req.type = 1;
    strncpy(gen_otp_req.txt, user_workspace, sizeof(gen_otp_req.txt));
    msgsnd(msg_id, &gen_otp_req, sizeof(gen_otp_req.txt), 0);
    printf("Workspace name sent to otp generator from log in: %s\n\n", gen_otp_req.txt);

    pid_t pid_otp = fork();
    if (pid_otp == 0) {
        struct msg otp_code;
        msgrcv(msg_id, &otp_code, sizeof(otp_code.txt), 1, 0);
        printf("OTP generator received workspace name from log in: %s\n\n", otp_code.txt);

        pid_t otp_code_val = getpid();
        snprintf(otp_code.txt, sizeof(otp_code.txt), "%d", otp_code_val);

        otp_code.type = 2;
        msgsnd(msg_id, &otp_code, sizeof(otp_code.txt), 0);
        printf("OTP sent to log in from OTP generator: %s\n", otp_code.txt);

        otp_code.type = 3;
        msgsnd(msg_id, &otp_code, sizeof(otp_code.txt), 0);
        printf("OTP sent to mail from OTP generator: %s\n\n", otp_code.txt);

        pid_t pid_mail = fork();
        if (pid_mail == 0) {
            struct msg mail_msg;
            msgrcv(msg_id, &mail_msg, sizeof(mail_msg.txt), 3, 0);
            printf("Mail received OTP from OTP generator: %s\n", mail_msg.txt);

            mail_msg.type = 4;
            msgsnd(msg_id, &mail_msg, sizeof(mail_msg.txt), 0);
            printf("OTP sent to log in from mail: %s\n", mail_msg.txt);

            exit(0);
        } else {
            wait(NULL);
            exit(0);
        }
    } else {
        wait(NULL);

        struct msg otp_verify;
        msgrcv(msg_id, &otp_verify, sizeof(otp_verify.txt), 2, 0);
        printf("Log in received OTP from OTP generator: %s\n", otp_verify.txt);

        struct msg mail_staus;
        msgrcv(msg_id, &mail_staus, sizeof(mail_staus.txt), 4, 0);
        printf("Log in received OTP from mail: %s\n", mail_staus.txt);

        if (strcmp(otp_verify.txt, mail_staus.txt) == 0) {
            printf("OTP Verified\n");
        } else {
            printf("OTP Incorrect\n");
        }

        msgctl(msg_id, IPC_RMID, NULL);
    }
    return 0;
}
