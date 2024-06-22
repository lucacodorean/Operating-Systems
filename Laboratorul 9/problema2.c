#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define NR_CUSTOMERS 20
#define NR_CHAIRS     3

sem_t lock, customer, barber, finishedClient;
pthread_mutex_t mutex;
int waitingCustomers = 0, barberChair = 0, doneCustomers = 0, done = 0;

void* barber_thread_function(void* args) {

    for(;;) {
        sem_post(&customer);
        sem_wait(&barber);
        printf("[B] Serving customer %d\n", barberChair);
        usleep(1000);

        printf("[B] Finalized customer %d\n", barberChair);
        sem_post(&finishedClient);

        pthread_mutex_lock(&mutex);
        if(done) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        pthread_mutex_unlock(&mutex);
    }
    
    return NULL;
}

void* customer_thread_function(void* args) {
    
    int myId = (int)(ssize_t)args;
    int tooBusy = 0;

    usleep(1000 * rand() % 20);
    printf("[C%02d] Entering the barber shop\n", myId);

    sem_wait(&lock);
    if(waitingCustomers < NR_CHAIRS) {
        ++waitingCustomers;
        printf("[C%02d] %d customer(s) waiting\n", myId, waitingCustomers);
    } 
    else {
        tooBusy = 1;
        printf("[C%02d] Too busy, will come back another day.\n", myId);
    }
    sem_post(&lock);

    if(!tooBusy) {
        sem_wait(&customer);

        sem_wait(&lock);
        --waitingCustomers;
        sem_post(&lock);

        pthread_mutex_lock(&mutex);
            doneCustomers++;
            if(doneCustomers == NR_CUSTOMERS) { done = 1; }
        pthread_mutex_unlock(&mutex);

        barberChair = myId;
        sem_post(&barber);
        printf("[C%02d] being served\n", myId);
        usleep(1000);

        sem_wait(&finishedClient);
        printf("[C%02d] The client is aware of his new cut and now he may leave.\n", myId);
    }

    return NULL;

}

int main(void) {

    int i;
    pthread_t tidC[NR_CUSTOMERS], tidB;
    srand(time(NULL));

    pthread_mutex_init(&mutex, NULL);
    sem_init(&lock, 0, 1);
    sem_init(&customer, 0, 0);
    sem_init(&barber, 0, 0);
    sem_init(&finishedClient, 0, 0);

    pthread_create(&tidB, NULL, barber_thread_function, NULL);
    for(int i = 0; i<NR_CUSTOMERS; i++) {
        pthread_create(&tidC[i], NULL, customer_thread_function, (void*)(ssize_t)(i+1));
    }

    for(int i = 0; i<NR_CUSTOMERS; i++) {
        pthread_join(tidC[i], NULL);
    }

    pthread_cancel(tidB);
    pthread_join(tidB, NULL);

    sem_destroy(&lock);
    sem_destroy(&customer);
    sem_destroy(&barber);
    sem_destroy(&finishedClient);
    pthread_mutex_destroy(&mutex);
    return 0;
}