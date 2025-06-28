//factorial and prime number generation using fork() and wait() 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// Function to check if a number is prime
int is_prime(int n) {
    if (n < 2) return 0;
    for (int i = 2; i*i <= n; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

int main() {
    int n;
    printf("Enter a number: ");
    scanf("%d", &n);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // First child: factorial
        FILE *f = fopen("factorial.txt", "w");
        if (!f) exit(1);
        unsigned long long fact = 1;
        for (int i = 2; i <= n; i++) fact *= i;
        fprintf(f, "%llu", fact);
        fclose(f);
        printf("Factorial saved to file: %llu\n", fact);
        exit(0);
    } else {
        pid_t pid2 = fork();
        if (pid2 == 0) {
            // Second child: primes
            FILE *f = fopen("primes.txt", "w");
            if (!f) exit(1);
            int first = 1;
            for (int i = 2; i <= n; i++) {
                if (is_prime(i)) {
                    if (!first) fprintf(f, ", ");
                    fprintf(f, "%d", i);
                    first = 0;
                }
            }
            fclose(f);
            printf("Primes saved to file: ");
            first = 1;
            for (int i = 2; i <= n; i++) {
                if (is_prime(i)) {
                    if (!first) printf(", ");
                    printf("%d", i);
                    first = 0;
                }
            }
            printf("\n");
            exit(0);
        } else {
            // Parent process
            wait(NULL);
            wait(NULL);

            // Read factorial
            FILE *f1 = fopen("factorial.txt", "r");
            unsigned long long fact;
            fscanf(f1, "%llu", &fact);
            fclose(f1);
            printf("Parent read the factorial: %llu\n", fact);

            // Read primes
            FILE *f2 = fopen("primes.txt", "r");
            char primes[256];
            fgets(primes, sizeof(primes), f2);
            fclose(f2);
            printf("Parent read the primes: %s\n", primes);
        }
    }
    return 0;
}