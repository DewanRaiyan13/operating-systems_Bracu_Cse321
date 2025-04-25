#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int count = argc - 1;
    int numbers[count];

    for (int i = 1; i < argc; i++) {
        numbers[i - 1] = atoi(argv[i]);
    }

    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            if (numbers[i] < numbers[j]) {
                int temp = numbers[i];
                numbers[i] = numbers[j];
                numbers[j] = temp;
            }
        }
    }

    printf("Sorted order: ");
    for (int i = 0; i < count; i++) {
        printf("%d", numbers[i]);
        if (i < count - 1) {
            printf(" "); 
        }
    }
    printf("\n");

    return 0;
}

