#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define NUM_STD 10
#define MAX_SEATS 3

sem_t std_sem;
sem_t tutors_sem;
sem_t con_done_sem;
pthread_mutex_t mutex;

int in_queue = 0;
int attended = 0;

void *std_process(void *arg) {
    int idx = *(int *)arg;
    usleep(rand() % 500000);

    pthread_mutex_lock(&mutex);
    if (in_queue < MAX_SEATS) {
        printf("Student %d started waiting for consultation \n", idx);
        in_queue++;
        pthread_mutex_unlock(&mutex);

        sem_post(&std_sem);
        sem_wait(&tutors_sem);

        pthread_mutex_lock(&mutex);
        in_queue--;
        printf("A waiting student started getting consultation \n");
        printf("Number of students now waiting: %d \n", in_queue);
        pthread_mutex_unlock(&mutex);

        printf("ST giving consultation \n");
        printf("Student %d is getting consultation \n", idx);
        usleep(200000);

        printf("Student %d finished getting consultation and left \n", idx);

        pthread_mutex_lock(&mutex);
        attended++;
        printf("Number of served students: %d \n", attended);
        pthread_mutex_unlock(&mutex);

        sem_post(&con_done_sem);
    } else {
        printf("No chairs remaining in lobby. Student %d Leaving..... \n", idx);
        attended++;
        printf("Number of served students: %d \n", attended);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void *tutors_process(void *arg) {
    while (1) {
        sem_wait(&std_sem);

        pthread_mutex_lock(&mutex);
        if (attended >= NUM_STD) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        sem_post(&tutors_sem);
        sem_wait(&con_done_sem);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t students[NUM_STD], tutor;
    int ids[NUM_STD];

    srand(time(NULL));

    sem_init(&std_sem, 0, 0);
    sem_init(&tutors_sem, 0, 0);
    sem_init(&con_done_sem, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&tutor, NULL, tutors_process, NULL);

    for (int i = 0; i < NUM_STD; i++) {
        ids[i] = i;
        pthread_create(&students[i], NULL, std_process, &ids[i]);
        usleep(100000);
    }

    for (int i = 0; i < NUM_STD; i++) {
        pthread_join(students[i], NULL);
    }
    pthread_join(tutor, NULL);

    sem_destroy(&std_sem);
    sem_destroy(&tutors_sem);
    sem_destroy(&con_done_sem);
    pthread_mutex_destroy(&mutex);

    return 0;
}

