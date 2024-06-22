#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

void* limited_area(void* unused);
int   nrThreads = 0;

typedef struct {
    sem_t* semaphore;
} THREAD_STRUCTURE;

int main(int argc, char** argv) {
    
    if(argc < 1) return 0;
    int N = -1;
    sscanf(argv[1], "%d", &N);

    pthread_t ids[16];
    THREAD_STRUCTURE threads_data[16];

    sem_t* semaphore = NULL;
    semaphore = sem_open("/problema2_semaphore", O_CREAT, 0644, N);

    for(size_t i = 0; i<16; i++) {
        threads_data[i].semaphore = semaphore;
        pthread_create(&ids[i], NULL, limited_area, &threads_data[i]); 
    }

    for(size_t i = 0; i<16; i++) pthread_join(ids[i], NULL);
    sem_unlink("/problema2_semaphore");
    return 0;
}

void* limited_area(void* unused) {

    THREAD_STRUCTURE* actual_value = (THREAD_STRUCTURE*)unused;

    sem_wait(actual_value->semaphore);
    nrThreads++;
    usleep(100);
    printf("The number of threads in the limited area is: %d\n", nrThreads);
    nrThreads--;
    sem_post(actual_value->semaphore);

    return NULL;
}
