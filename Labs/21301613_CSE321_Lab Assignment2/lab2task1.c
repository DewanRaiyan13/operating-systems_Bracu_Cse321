#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_FIB 40

int *fib_array = NULL;
int fib_count = 0; 

int *in_keys = NULL;
int *out_values = NULL;
int search_count = 0;

void *fibo_generate(void *arg) {
    fib_array = (int *)malloc((fib_count + 1) * sizeof(int));
    if (!fib_array) pthread_exit(NULL);

    if (fib_count >= 0) fib_array[0] = 0;
    if (fib_count >= 1) fib_array[1] = 1;
    for (int i = 2; i <= fib_count; i++) {
        fib_array[i] = fib_array[i-1] + fib_array[i-2];
    }
    pthread_exit(NULL);
}

void *find_fibonacci(void *arg) {
    for (int i = 0; i < search_count; i++) {
        int idx = in_keys[i];
        if (idx >= 0 && idx <= fib_count) {
            out_values[i] = fib_array[idx];
        } else {
            out_values[i] = -1;
        }
    }
    pthread_exit(NULL);
}

int main() {
    printf("Enter the term of fibonacci sequence:\n");
    scanf("%d", &fib_count);
    if (fib_count < 0 || fib_count > MAX_FIB) {
        printf("Term must be between 0 and 40.\n");
        return 1;
    }

    pthread_t fibo_thread;
    pthread_create(&fibo_thread, NULL, fibo_generate, NULL);
    pthread_join(fibo_thread, NULL);

    printf("Fibonacci sequence:\n");
    for (int i = 0; i <= fib_count; i++) {
        printf("a[%d] = %d\n", i, fib_array[i]);
    }

    printf("How many numbers are you willing to search?:\n");
    scanf("%d", &search_count);
    if (search_count <= 0) {
        printf("Number of searches must be greater than 0.\n");
        free(fib_array);
        return 1;
    }

    in_keys = (int *)malloc(search_count * sizeof(int));
    out_values = (int *)malloc(search_count * sizeof(int));
    for (int i = 0; i < search_count; i++) {
        printf("Enter search %d:\n", i + 1);
        scanf("%d", &in_keys[i]);
    }

    pthread_t find_thread;
    pthread_create(&find_thread, NULL, find_fibonacci, NULL);
    pthread_join(find_thread, NULL);

    for (int i = 0; i < search_count; i++) {
        printf("result of search #%d = %d\n", i + 1, out_values[i]);
    }

    free(fib_array);
    free(in_keys);
    free(out_values);
    return 0;
}
