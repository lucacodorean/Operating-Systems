#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#ifndef __THREAD_STRUCTURE__
    #define __THREAD_STRUCTURE__
    #define CLOSED_THREAD_ID        -1

    typedef struct {
        short id;
    } THREAD_STRUCTURE;

    void* thread_function(void*);

#endif

#define TRUE 1


int main(int argc, char** argv) {
    
    pthread_t thread_ids[9];
    THREAD_STRUCTURE threads[9];

    for(size_t i = 0; i<9; i++) {
        threads[i].id = i + 1;
        if(pthread_create(&thread_ids[i], NULL, thread_function, &threads[i]) != 0) {
            perror("Thread can't be created.");
            return -1;
        }
    }

    for(short i = 0, number; i<9; ) {
        scanf("%hd", &number);
        if(number > 9 || number < 1) {
            printf("Numarul introdus trebuie sa fie mai mic decat 9 si mai mare decat 1.\n");
            continue;
        }

        for(int j = 0; j<9; j++) {
            if(threads[j].id != number) continue;

            printf("Threadul cu meta-data: %hd (thread id: %ld) a fost anulat.\n", threads[j].id, thread_ids[j]);
            
            pthread_cancel(thread_ids[j]);
            threads[j].id = CLOSED_THREAD_ID;
            i++;
        }
    }

    printf("Main thread will be closed.");
    return 0;
}

void* thread_function(void* parameter) {
    THREAD_STRUCTURE* number = (THREAD_STRUCTURE*)parameter;

    for(;;) {
        sleep(rand() % 5 + 1);
        printf("Thread-ul cu meta-data id: %hd s-a incheiat.\n", number->id);
        break;
    }

    return NULL;
}