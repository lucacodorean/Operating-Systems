#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __THREAD_STRUCTURE__

    #define __THREAD_STRUCTURE__

    typedef struct {
        size_t  from;
        size_t  to;
    } THREAD_STRUCTURE;

    void  init_threads(const size_t, const size_t, pthread_t*, THREAD_STRUCTURE*);
    void* threadFn(void*);
#endif

size_t get_number_of_ones(size_t);
void   display_result(const size_t, const size_t, pthread_t*);

int main(int argc, char** argv) {

    printf("Introduceti valoarea lui ARR_SIZE: ");
    size_t ARR_SIZE; scanf("%lu", &ARR_SIZE);

    const size_t NUMBER_OF_THREADS = (size_t)sqrt(ARR_SIZE);
    printf("%lu threaduri vor fi folosite pentru %lu elemente.\n", NUMBER_OF_THREADS, ARR_SIZE);

    pthread_t threads_id[NUMBER_OF_THREADS];
    THREAD_STRUCTURE threads[NUMBER_OF_THREADS];

    init_threads(ARR_SIZE, NUMBER_OF_THREADS, threads_id, threads);
    display_result(ARR_SIZE, NUMBER_OF_THREADS, threads_id);

    return 0;
}   

size_t get_number_of_ones(size_t number) {

    size_t result = 0;
    while(number > 0) {
        result += (number%10 == 1);
        number/=10;
    }

    return result;
}

void* threadFn(void* parameter) {
    THREAD_STRUCTURE* actual_value = (THREAD_STRUCTURE*)parameter;

    size_t ones = 0;
    for(size_t current_number = actual_value->from; current_number < actual_value->to; current_number++) {
        ones += get_number_of_ones(current_number);
    }

    return (void*)(long)ones;
}

void init_threads(const size_t ARR_SIZE, const size_t NUMBER_OF_THREADS, pthread_t* threads_id, THREAD_STRUCTURE* threads) {
    for(size_t i = 0; i<NUMBER_OF_THREADS; i++) {
        threads[i].from = (i == 0) ? 1 : threads[i-1].to + 1;
        threads[i].to   = threads[i].from + ARR_SIZE / NUMBER_OF_THREADS;
        printf("%ld ", threads[i].to - threads[i].from + 1);
        if(i < ARR_SIZE % NUMBER_OF_THREADS) threads[i].to++;

        if(pthread_create(&threads_id[i], NULL, threadFn, &threads[i]) != 0) {
            perror("Error creating the thread.");
            return;
        }
    }

    return;
}

void display_result(const size_t ARR_SIZE, const size_t NUMBER_OF_THREADS, pthread_t* threads_id) {
    void*  current_thread_result = NULL;
    size_t result = 0;
    for(size_t i = 0; i<NUMBER_OF_THREADS; i++) {
        pthread_join(threads_id[i], &current_thread_result);
        result += (size_t)(long)current_thread_result;
    }
    
    printf("In intervalul [1, %ld] se gasesc %ld valori de 1, in scrierea numerelor.", ARR_SIZE, result);
    return;
}