#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    char buff[256];
    FILE *fp;

    if (argc < 2) {
        printf("Usage: %s <password>\n", argv[0]);
        return 1;
    }

    char *password = argv[1];
    int a, status;
    a = fork();

    if (a == 0) {
        fp = fopen("password.txt", "w");
        if (fp == NULL) {
            perror("Error opening file for writing");
            return 1;
        }
        fprintf(fp, "%s", password);
        fclose(fp);
        printf("Password saved to a file.\n");
    } else if (a > 0) {
        wait(&status);
        fp = fopen("password.txt", "r");
        if (fp == NULL) {
            perror("Error opening file for reading");
            return 1;
        }
        fgets(buff, sizeof(buff), fp);
        fclose(fp);
        printf("Password read from file: %s\n", buff);

        bool contains = false;
        for (int i = 0; buff[i] != '\0'; i++) {
            char ch = tolower(buff[i]);
            if (ch == 'a' || ch == 'b' || ch == 'c' || ch == 'd') {
                contains = true;
                break;
            }
        }

        if (contains) {
            printf("Password contains A or B or C or D: Yes\n");
        } else {
            printf("Password contains A or B or C or D: No\n");
        }
    } else {
        perror("Fork failed");
        return 1;
    }

    return 0;
}
