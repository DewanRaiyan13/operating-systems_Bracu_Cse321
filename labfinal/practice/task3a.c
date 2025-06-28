// Description: ATM simulation with multiple customers using mutex for synchronization
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int balance = 1000;
pthread_mutex_t atm_mutex;

void* customer(void* arg) {
    int id = ((int*)arg)[0];
    int type = ((int*)arg)[1]; // 0 = withdraw, 1 = deposit
    int amount = ((int*)arg)[2];

    pthread_mutex_lock(&atm_mutex);

    if (type == 0) { // Withdraw
        printf("Customer %d is withdrawing $%d.\n", id, amount);
        if (amount > balance) {
            printf("Withdrawal denied. Insufficient funds.\n");
        } else {
            balance -= amount;
            printf("Remaining balance: $%d.\n", balance);
        }
    } else { // Deposit
        printf("Customer %d is depositing $%d.\n", id, amount);
        balance += amount;
        printf("Updated balance: $%d.\n", balance);
    }

    pthread_mutex_unlock(&atm_mutex);
    return NULL;
}

int main() {
    pthread_t t1, t2, t3;
    pthread_mutex_init(&atm_mutex, NULL);

    int args1[3] = {1, 0, 100}; // Customer 1 withdraws $100
    int args2[3] = {2, 1, 200}; // Customer 2 deposits $200
    int args3[3] = {3, 0, 500}; // Customer 3 withdraws $500

    pthread_create(&t1, NULL, customer, args1);
    pthread_join(t1, NULL);

    pthread_create(&t2, NULL, customer, args2);
    pthread_join(t2, NULL);

    pthread_create(&t3, NULL, customer, args3);
    pthread_join(t3, NULL);

    pthread_mutex_destroy(&atm_mutex);
    return 0;
}