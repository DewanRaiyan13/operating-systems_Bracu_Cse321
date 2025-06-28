//multiple customers trying to buy chips from a vending machine
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int stock = 5;
pthread_mutex_t vending_mutex;

void* customer(void* arg) {
    int id = *(int*)arg;

    pthread_mutex_lock(&vending_mutex);

    if (stock > 0) {
        printf("Customer %d purchased Chips.\n", id);
        stock--;
        printf("Remaining stock: %d.\n", stock);
    } else {
        printf("Customer %d could not purchase Chips. Stock is empty.\n", id);
    }

    pthread_mutex_unlock(&vending_mutex);
    return NULL;
}

int main() {
    pthread_t t1, t2, t3;
    int id1 = 1, id2 = 2, id3 = 3;

    pthread_mutex_init(&vending_mutex, NULL);

    pthread_create(&t1, NULL, customer, &id1);
    pthread_join(t1, NULL);

    pthread_create(&t2, NULL, customer, &id2);
    pthread_join(t2, NULL);

    pthread_create(&t3, NULL, customer, &id3);
    pthread_join(t3, NULL);

    pthread_mutex_destroy(&vending_mutex);
    return 0;
}