#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

void* thread_function(void*);
long  count = 0;

typedef struct {
    long id;
    long M;
    sem_t* semaphore;
} THREAD_STRUCTURE;

int main(void) {

    int N = 4;
    long M = 1000;
    pthread_t ids[N];
    THREAD_STRUCTURE threads_data[N];

    sem_t* semaphore = NULL;
    semaphore = sem_open("/problema1_semaphore", O_CREAT, 0644, 1);

    for(size_t i = 0; i<N; i++) {
        threads_data[i].id = i;
        threads_data[i].M =  M;
        threads_data[i].semaphore = semaphore;
        pthread_create(&ids[i], NULL, thread_function, &threads_data[i]); 
    }

    for(size_t i = 0; i<N; i++) pthread_join(ids[i], NULL);

    printf("\n%ld\n", count);
    sem_unlink("/problema1_semaphore");
    return 0;
}


void* thread_function(void* args) {

    THREAD_STRUCTURE* actual_data = (THREAD_STRUCTURE*)args;
    long aux;

    for(size_t i = 0; i<actual_data->M; i++) {
        sem_wait(actual_data->semaphore);
        aux = count;
        aux++;
        usleep(random() % 10);
        count = aux;
        sem_post(actual_data->semaphore);
    }
    return NULL;
}