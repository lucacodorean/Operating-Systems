#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <string.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>

#ifndef __ERROR_IDENTIFIER__
    #define __ERROR_IDENTIFIER__

    enum STATUS_CODES{
        SUCCESS                 =  0,
        GENERAL_ERROR           = -1,
        READ_ERROR              = -2,
        MEMORY_ALLOC_ERROR      = -3,
        FILE_ERROR              = -4,
        PARAMETER_NOT_ACCEPTED  = -5,
        DIRECTORY_PATH_ERROR    = -6,
        LOCK_ERROR              = -7,
    };

    #define PRINT_ERROR(X) { printf("Error code: %d at line %d.\n", X, __LINE__); }
#endif

#ifndef __SOLVERS__
    #define __SOLVERS__

    #define P4_THREADS_MAX       4
    #define P3_THREADS_MAX      38
    #define P2_THREADS_MAX       6
    #define P4_SPECIAL_THREAD_ID 3
    #define P3_MAX_THREAD_COUNT  6
    #define P3_SPECIAL_THREAD_ID 13
    #define CHILD                0

    typedef struct __attribute__((packed)) {
        size_t                  process_id;
        size_t                  thread_id; 
        sem_t*                  this_semaphore;
        sem_t*                  next_semaphore;
    } NO2_THREAD_STRUCTURE;

    typedef struct __attribute__((packed)) {
        size_t                  process_id;
        size_t                  thread_id;
        sem_t*                  semaphore;
        pthread_mutex_t*        lock;
        pthread_cond_t*         cond;
        sem_t*                  barrier;
    } NO3_THREAD_STRUCTURE;

    void*  thread_4_function(void*);
    void*  thread_3_function(void*);
    void*  thread_3_function_special(void*);
    void*  thread_2_function(void*);
    void*  thread_2_function_thread_2(void*);
    void*  thread_2_function_thread_3(void*);
    void   set_4th_process_threads();
    void   set_3rd_process_threads();
    void   set_2nd_process_threads();
    void   solver();

    int T13_IS_ACTIVE = 0, P3_CURRENT_COUNT = 0;
    sem_t *shared_4_2_semaphore_t2_t3 = NULL, *shared_4_2_semaphore_t3_t3 = NULL;

#endif

int main(int argc, char** argv) {

    shared_4_2_semaphore_t3_t3 = sem_open("/semafor_t3_t3", O_CREAT, 0644, 0);
    shared_4_2_semaphore_t2_t3 = sem_open("/semafor_t2_t3", O_CREAT, 0644, 0);

    solver();

    sem_destroy(shared_4_2_semaphore_t2_t3);
    sem_destroy(shared_4_2_semaphore_t3_t3);
    return 0;
}

void* thread_4_function(void* args) {
    if(args == NULL) return NULL;

    NO2_THREAD_STRUCTURE* actual_value = (NO2_THREAD_STRUCTURE*)args;

    if(actual_value->thread_id == P4_SPECIAL_THREAD_ID) sem_wait(shared_4_2_semaphore_t2_t3);
    info(BEGIN, actual_value->process_id, actual_value->thread_id);
    sem_wait(actual_value->this_semaphore);

    info(END, actual_value->process_id, actual_value->thread_id);

    if(actual_value->thread_id == P4_SPECIAL_THREAD_ID) sem_post(shared_4_2_semaphore_t3_t3);

    sem_post(actual_value->next_semaphore);

    pthread_exit(0);
    return NULL;
}

/*
    Avem 2 conditii:
        Daca P3_COUNTER ajunge la 6, ar trebui sa ii dea signal la cond2 care sa permita stergerea lui P3_SPECIAL;
        P3_SPECIAL, ar trebui, dupa stergere, sa dea broadcast la celelalte threaduri sa se execute.
*/
void* thread_3_function(void* args) {
    if(args == NULL) return NULL;

    NO3_THREAD_STRUCTURE* actual_value = (NO3_THREAD_STRUCTURE*)args;
    sem_wait(actual_value->semaphore);
    info(BEGIN, actual_value->process_id, actual_value->thread_id);
    P3_CURRENT_COUNT++;

    pthread_mutex_lock(actual_value->lock);
    if(P3_CURRENT_COUNT == P3_MAX_THREAD_COUNT-1 && T13_IS_ACTIVE == false) {
        pthread_cond_signal(actual_value->cond);
    }
    info(END, actual_value->process_id, actual_value->thread_id);
    P3_CURRENT_COUNT--;
    pthread_mutex_unlock(actual_value->lock);
    
    sem_post(actual_value->semaphore);
    return NULL;
}

void* thread_3_function_special(void* args) {
    if(args == NULL) return NULL;

    NO3_THREAD_STRUCTURE* actual_value = (NO3_THREAD_STRUCTURE*)args;
    pthread_mutex_lock(actual_value->lock);
    while(P3_CURRENT_COUNT == P3_MAX_THREAD_COUNT-1) {
        pthread_cond_wait(actual_value->cond, actual_value->lock);
    }
    P3_CURRENT_COUNT++;
    info(BEGIN, actual_value->process_id, actual_value->thread_id);
    T13_IS_ACTIVE = true;
    info(END, actual_value->process_id, actual_value->thread_id);
    P3_CURRENT_COUNT--;
    pthread_mutex_unlock(actual_value->lock);
    return NULL;
}

void* thread_2_function(void* args) {
    if(args == NULL) return NULL;

    NO3_THREAD_STRUCTURE* actual_value = (NO3_THREAD_STRUCTURE*)args;

    info(BEGIN, actual_value->process_id, actual_value->thread_id);
    info(END, actual_value->process_id, actual_value->thread_id);
    pthread_exit(0);
    return NULL;
}

void* thread_2_function_thread_3(void* args) {
    if(args == NULL) return NULL;

    NO3_THREAD_STRUCTURE* actual_value = (NO3_THREAD_STRUCTURE*)args;

    sem_wait(shared_4_2_semaphore_t3_t3);
    info(BEGIN, actual_value->process_id, actual_value->thread_id);
    info(END, actual_value->process_id, actual_value->thread_id);
    pthread_exit(0);
    return NULL;
}

void* thread_2_function_thread_2(void* args) {
    if(args == NULL) return NULL;

    NO3_THREAD_STRUCTURE* actual_value = (NO3_THREAD_STRUCTURE*)args;
    
    info(BEGIN, actual_value->process_id, actual_value->thread_id);
    info(END,   actual_value->process_id, actual_value->thread_id);
    sem_post(shared_4_2_semaphore_t2_t3);

    pthread_exit(0);
    return NULL;
}

void set_4th_process_threads() {
    pthread_t* thread_ids = (pthread_t*)malloc(P4_THREADS_MAX * sizeof(pthread_t));
    if(thread_ids == NULL) {
        free(thread_ids); thread_ids = NULL;
        PRINT_ERROR(MEMORY_ALLOC_ERROR);
        return;
    }
    memset(thread_ids, 0,  P4_THREADS_MAX * sizeof(pthread_t));

    NO2_THREAD_STRUCTURE *thread_data = (NO2_THREAD_STRUCTURE*)malloc(P4_THREADS_MAX * sizeof(NO2_THREAD_STRUCTURE));
    if(thread_data == NULL) {
        free(thread_ids); thread_ids = NULL;
        free(thread_data); thread_data = NULL;
        PRINT_ERROR(MEMORY_ALLOC_ERROR);
        return;
    }
    memset(thread_data, 0, P4_THREADS_MAX * sizeof(NO2_THREAD_STRUCTURE));

    sem_t semaphores[P4_THREADS_MAX];
    for(size_t i=0; i<P4_THREADS_MAX; i++) sem_init(&semaphores[i], 0, 0);
    
    for(size_t i = 0; i<P4_THREADS_MAX; i++) { 
        thread_data[i].process_id =   4;
        thread_data[i].thread_id  = i+1;
        thread_data[i].this_semaphore = &semaphores[i];
        thread_data[i].next_semaphore = &semaphores[(i+1) % P4_THREADS_MAX];
    }
    
    pthread_create(&thread_ids[P4_THREADS_MAX-1], NULL, thread_4_function, &thread_data[P4_THREADS_MAX-1]);
    for(size_t i = 0; i<P4_THREADS_MAX-1; i++) {
        pthread_create(&thread_ids[i], NULL, thread_4_function, &thread_data[i]);
    }

    sem_post(&semaphores[0]);

    for(size_t i = 0; i<P4_THREADS_MAX; i++) pthread_join(thread_ids[i], NULL);
    for(size_t i = 0; i<P4_THREADS_MAX; i++) sem_destroy(&semaphores[i]);
    
    free(thread_data); thread_data = NULL;
    free(thread_ids); thread_ids = NULL;
    return;
}

void set_3rd_process_threads() {

    pthread_t* thread_ids = (pthread_t*)malloc(P3_THREADS_MAX * sizeof(pthread_t));
    if(thread_ids == NULL) {
        free(thread_ids); thread_ids = NULL;
        PRINT_ERROR(MEMORY_ALLOC_ERROR);
        return;
    }
    memset(thread_ids, 0,  P3_THREADS_MAX * sizeof(pthread_t));

    NO3_THREAD_STRUCTURE *thread_data = (NO3_THREAD_STRUCTURE*)malloc(P3_THREADS_MAX * sizeof(NO3_THREAD_STRUCTURE));
    if(thread_data == NULL) {
        free(thread_ids); thread_ids = NULL;
        free(thread_data); thread_data = NULL;
        PRINT_ERROR(MEMORY_ALLOC_ERROR);
        return;
    }
    memset(thread_data, 0, P3_THREADS_MAX * sizeof(NO3_THREAD_STRUCTURE));

    sem_t semaphore;
    sem_t barrier;
    pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    sem_init(&semaphore, 0, P3_MAX_THREAD_COUNT-1);
    sem_init(&barrier, 0, 0);

    for(size_t i = 0; i<P3_THREADS_MAX; i++) { 
        thread_data[i].process_id =   3;
        thread_data[i].thread_id  = i+1;
        thread_data[i].semaphore  = &semaphore;
        thread_data[i].lock       = &lock;
        thread_data[i].cond       = &cond;
        thread_data[i].barrier    = &barrier;
    }

    for(size_t i = 0; i<P3_THREADS_MAX; i++) {
        if(i == P3_SPECIAL_THREAD_ID-1) pthread_create(&thread_ids[i], NULL, thread_3_function_special, &thread_data[i]);
        else pthread_create(&thread_ids[i], NULL, thread_3_function, &thread_data[i]);
    }
    for(size_t i = 0; i<P3_THREADS_MAX; i++) pthread_join(thread_ids[i], NULL);

    sem_close(&semaphore);
    sem_close(&barrier);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    free(thread_data);  thread_data = NULL;
    free(thread_ids);   thread_ids = NULL;
    return;
}

void set_2nd_process_threads() {
    pthread_t* thread_ids = (pthread_t*)malloc(P2_THREADS_MAX * sizeof(pthread_t));
    if(thread_ids == NULL) {
        free(thread_ids); thread_ids = NULL;
        PRINT_ERROR(MEMORY_ALLOC_ERROR);
        return;
    }
    memset(thread_ids, 0,  P2_THREADS_MAX * sizeof(pthread_t));

    NO3_THREAD_STRUCTURE *thread_data = (NO3_THREAD_STRUCTURE*)malloc(P2_THREADS_MAX * sizeof(NO3_THREAD_STRUCTURE));
    if(thread_data == NULL) {
        free(thread_ids); thread_ids = NULL;
        free(thread_data); thread_data = NULL;
        PRINT_ERROR(MEMORY_ALLOC_ERROR);
        return;
    }
    memset(thread_data, 0, P2_THREADS_MAX * sizeof(NO3_THREAD_STRUCTURE));

    
    for(size_t i = 0; i<P2_THREADS_MAX; i++) { 
        thread_data[i].process_id =   2;
        thread_data[i].thread_id  = i+1;
    }

    for(size_t i = 0; i<P2_THREADS_MAX; i++) {
        if(i == 1)      pthread_create(&thread_ids[i], NULL, thread_2_function_thread_2, &thread_data[i]);
        else if(i==2)   pthread_create(&thread_ids[i], NULL, thread_2_function_thread_3, &thread_data[i]);
        else            pthread_create(&thread_ids[i], NULL, thread_2_function, &thread_data[i]);
    }

    for(size_t i = 0; i<P2_THREADS_MAX; i++) pthread_join(thread_ids[i], NULL);

    free(thread_data); thread_data = NULL;
    free(thread_ids); thread_ids = NULL;
    return;
}

void solver() {
    init();
    info(BEGIN, 1, 0);

    pid_t P2 = fork();
    if(P2 == CHILD) {
        info(BEGIN, 2, 0);
        pid_t P5 = fork();
        if(P5 == CHILD) {
            info(BEGIN, 5, 0);
            info(END, 5, 0);
            exit(0);
        }

        waitpid(P5, NULL, 0);
        set_2nd_process_threads();
        info(END, 2, 0);
        exit(0);
    }

    pid_t P3 = fork();
    if(P3 == CHILD) {
        info(BEGIN, 3, 0);
        pid_t P4 = fork();
        if(P4 == CHILD) {
            info(BEGIN, 4, 0);
            pid_t P6 = fork();
            if(P6 == CHILD) {
                info(BEGIN, 6, 0);
                info(END, 6, 0);
                exit(0);
            }

            pid_t P7 = fork();
            if(P7 == CHILD) {
                info(BEGIN, 7, 0);
                pid_t P8 = fork();
                
                if(P8 == CHILD) {
                    info(BEGIN, 8, 0);
                    info(END, 8, 0);
                    exit(0);
                }

                pid_t P9 = fork();
                if(P9 == CHILD) {
                    info(BEGIN, 9, 0);
                    info(END, 9, 0);
                    exit(0);
                }

                waitpid(P8, NULL, 0);
                waitpid(P9, NULL, 0);
                info(END, 7, 0);
                exit(0);
            }

            waitpid(P6, NULL, 0);
            waitpid(P7, NULL, 0);
            set_4th_process_threads();
            info(END, 4, 0);
            exit(0);
        }

        waitpid(P4, NULL, 0);
        set_3rd_process_threads();

        info(END, 3, 0);
        exit(0);
    }

    waitpid(P2, NULL, 0);
    waitpid(P3, NULL, 0);
    info(END, 1, 0);
    return;
}