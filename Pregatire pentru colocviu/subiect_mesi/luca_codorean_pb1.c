#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

typedef struct {
    int first_lower_bound, second_lower_bound;
    int first_upper_bound, second_upper_bound;
    int id;
    pthread_mutex_t* mutex;
    sem_t* semaphore;
    sem_t* first_semaphore_wake, *second_semaphore_wake;
} THREAD_STRUCTURE;


void* thread_function(void*);
void assignd_data(THREAD_STRUCTURE* , int, int, int, int, int, pthread_mutex_t* );


/*
    0 first wakes  2 first
    2 first wakes  0 second
    0 second wakes 1 first
    1 first wakes 2 second
    2 second wakes 1 first
*/

int counter = 0;
int main(void) {

    const int n = 3;
    pthread_t thread_ids[n];
    THREAD_STRUCTURE thread_data[n];
    sem_t* semaphore0 = NULL, *semaphore1 = NULL, *semaphore2 = NULL;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 

    semaphore0 = sem_open("/SEM0", O_CREAT, 0644, 0);
    semaphore1 = sem_open("/SEM1", O_CREAT, 0644, 0);
    semaphore2 = sem_open("/SEM2", O_CREAT, 0644, 0);

    assignd_data(&thread_data[0], 0, 0, 10, 20,   31, &mutex);
    assignd_data(&thread_data[1], 1, 31, 76, 91, 100, &mutex);
    assignd_data(&thread_data[2], 2, 10, 20, 76,  91, &mutex);

    thread_data[0].semaphore             = semaphore0;
    thread_data[0].first_semaphore_wake  = semaphore2;
    thread_data[0].second_semaphore_wake = semaphore1;

    thread_data[1].semaphore             = semaphore1;
    thread_data[1].first_semaphore_wake  = semaphore2;
    thread_data[1].second_semaphore_wake = NULL;

    thread_data[2].semaphore             = semaphore2;
    thread_data[2].first_semaphore_wake  = semaphore0;
    thread_data[2].second_semaphore_wake = semaphore1;


    for(int i = 0; i<n; i++)
        pthread_create(&thread_ids[i], NULL, thread_function, &thread_data[i]);

    sem_post(semaphore0);

    for(int i = 0; i<n; i++) 
        pthread_join(thread_ids[i], NULL);

    printf("[MAIN]: Valoarea lui counter este: %d.", counter);

    return 0;
}

void assignd_data(THREAD_STRUCTURE* th, int id, int first_lower, int first_upper, int second_lower, int second_upper, pthread_mutex_t* mutex) {
    if(th == NULL) return;

    th->id = id;
    th->first_lower_bound = first_lower;
    th->first_upper_bound = first_upper;
    th->second_lower_bound = second_lower;
    th->second_upper_bound = second_upper;
    th->mutex = mutex;
}

void* thread_function(void* args) {
    THREAD_STRUCTURE* actual_args = (THREAD_STRUCTURE*)args;
    sem_wait(actual_args->semaphore);

    pthread_mutex_lock(actual_args->mutex);
    printf("[T%d] has the mutex now. It counts from %d to %d.\n", actual_args->id, actual_args->first_lower_bound, actual_args->first_upper_bound);
    for(int i = actual_args->first_lower_bound; i<actual_args->first_upper_bound; i++) counter++;
    pthread_mutex_unlock(actual_args->mutex);

    sem_post(actual_args->first_semaphore_wake);
    sem_wait(actual_args->semaphore);

    pthread_mutex_lock(actual_args->mutex);
    printf("[T%d] has the mutex now. It counts from %d to %d.\n", actual_args->id, actual_args->second_lower_bound, actual_args->second_upper_bound);
    for(int i = actual_args->second_lower_bound; i<actual_args->second_upper_bound; i++) counter++;
    pthread_mutex_unlock(actual_args->mutex);

    if(actual_args->second_semaphore_wake != NULL) sem_post(actual_args->second_semaphore_wake);
    return NULL;
}