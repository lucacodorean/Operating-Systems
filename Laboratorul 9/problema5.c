#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define NR_SMOKERS    3

sem_t ingrendients[3], ingrendientsProcessed;
pthread_mutex_t generatorMutex;

int first_ingrendient_index, second_ingrendient_index;

void* dealer_thread(void* args) {

    for(;;) {
        sem_wait(&ingrendientsProcessed);

        pthread_mutex_lock(&generatorMutex);
        first_ingrendient_index  = rand() % 3;
        second_ingrendient_index = rand() % 3;

        if(second_ingrendient_index == first_ingrendient_index) {
            while(second_ingrendient_index == first_ingrendient_index) {
                second_ingrendient_index = rand() % 3;
            }
        }
        pthread_mutex_unlock(&generatorMutex);

        printf("[D] Dealer generates ingrendients: %d and %d.\n", first_ingrendient_index, second_ingrendient_index);
        sem_post(&ingrendients[first_ingrendient_index]);
        sem_post(&ingrendients[second_ingrendient_index]);
    }


    return NULL;
}


/*
    am avea 3 pusheri, cate unul pentru fiecare set de ingrendiente generate 
        ingrendientele: (0,1), (1,2), (2,0)
    trezeste smokerul care asteapta dupa cele doua ingrendiente
*/

void* pusher_thread(void* args) {
    
}

/*
*/
void* smoker_thread(void* args) {

    for(;;) {
        usleep(100000);
        sem_post(&ingrendientsProcessed);
    }

    return NULL;
}

int main(void) {

    pthread_t smokers[NR_SMOKERS], dealer;

    srand(time(NULL));

    sem_init(&ingrendientsProcessed, 0, 1);
    for(int i = 0; i<3; i++) sem_init(&ingrendients[i], 0, 0);


    pthread_create(&dealer, NULL, dealer_thread, NULL);
    for(int i = 0; i<NR_SMOKERS; i++) {
        pthread_create(&smokers[i], NULL, smoker_thread, (void*)(ssize_t)i+1);
    }

    for(int i = 0; i<NR_SMOKERS; i++) pthread_join(smokers[i], NULL);
    pthread_cancel(dealer);
    pthread_join(dealer, NULL);

    sem_destroy(&ingrendientsProcessed);
    pthread_mutex_destroy(&generatorMutex);
    for(int i = 0; i<3; i++) sem_destroy(&ingrendients[i]);
    return 0;
}
