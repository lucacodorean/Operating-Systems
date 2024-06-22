#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

void* thread_function(void*);

#define TRUE 1

typedef struct th_s {
    unsigned int id;
    sem_t* semaphore;
    sem_t* vecin;
} THREAD_STRUCTURE;

int main(int argc, char** argv) {
    
    int N; scanf("%d", &N);

    pthread_t ids[N];
    THREAD_STRUCTURE threads_data[N];
    sem_t semafoare[N];

    for(size_t i = 0; i<N; i++)  sem_init(&semafoare[i], 0, 0);
    
    for(size_t i = 0; i<N; i++) {
        threads_data[i].id = i;
        threads_data[i].semaphore = &semafoare[i];
        threads_data[i].vecin = &semafoare[(i+1) % N ];
        pthread_create(&ids[i], NULL, thread_function, &threads_data[i]); 
    }

    sem_post(&semafoare[0]);
    for(size_t i = 0; i<N; i++) pthread_join(ids[i], NULL);

    return 0;
}

void* thread_function(void* args) {
    THREAD_STRUCTURE* actual_value = (THREAD_STRUCTURE*)args;

    while(TRUE) {
        sem_wait(actual_value->semaphore);
        printf("%d ", actual_value->id);
        fflush(stdout);
        sem_post(actual_value->vecin);
    }

    return NULL;
}
