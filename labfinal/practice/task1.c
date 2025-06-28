// greatest common divisor (GCD) algorithm:  uses the GCD algorithm.
#include <stdio.h>

int main() {
    int a, b, i, gcd = 1;
    printf("Enter two numbers: ");
    scanf("%d%*[, ]%d", &a, &b);

    int min = (a < b) ? a : b;
    for (i = 1; i <= min; i++) {
        if (a % i == 0 && b % i == 0) {
            gcd = i;
        }
    }

    printf("GCD is: %d\n", gcd);
    return 0;
}