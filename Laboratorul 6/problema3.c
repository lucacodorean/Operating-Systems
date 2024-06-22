#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#ifndef __THREAD_STRUCTURE__
    #define __THREAD_STRUCTURE__
    #define THREADS_MAX 500000

    void* threadFn(void*);
#endif

int main(void) {
    pthread_t thread_ids[THREADS_MAX];
    size_t cate = 0;

    while(pthread_create(&thread_ids[cate++], NULL, threadFn, NULL) == 0);
    for(size_t i = 0; i<cate; i++) pthread_cancel(thread_ids[i]); 

    printf("Se pot crea %ld thread-uri.", cate);
    return 0;
}

void* threadFn(void* parameter) {
    for(;;) sleep(1);
}