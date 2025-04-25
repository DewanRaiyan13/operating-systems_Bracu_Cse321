#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
    FILE *file_ptr = fopen(argv[1], "w");
    char input[300];

    scanf("%s", input);
    while (true) {
        bool continue_input = true;

        for (int i = 0; i < strlen(input) - 1; i++) {
            if (input[i] == '-' && input[i + 1] == '1') {
                continue_input = false;
                break;
            }
        }
        if (!continue_input) break;
        fprintf(file_ptr, "%s\n", input);
        scanf("%s", input);
    }

    fclose(file_ptr);
    return 0;
}

