#include <stdio.h>
#include <stdlib.h>

int main(int arg_count, char *arg_values[]) {

    int num_count = arg_count - 1;
    int numbers[num_count];

    for (int i = 0; i < num_count; i++) {
        numbers[i] = atoi(arg_values[i + 1]);
    }

    for (int i = 0; i < num_count; i++) {
        int value = numbers[i];
        if (value % 2 == 0)
            printf("%d : even\n", value);
        else
            printf("%d : odd\n", value);
    }

    return 0;
}

