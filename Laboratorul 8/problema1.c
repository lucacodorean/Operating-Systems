#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

typedef struct {
    pthread_mutex_t* lock_fetch;
    pthread_mutex_t* lock_write;
} THREAD_DATA;

typedef struct {
    int numar;
    int numar_divizori;
} RESULTS;

#define DONE -1

int         n, CURRENT_INDEX = 0, tasks[100000] = {0}, CURRENT_INDEX_TASKS = 0;
int         NR_THREADS = -1,  FINISHED = 0;
RESULTS     rezultate[100000];

int     numar_divizori(int);
int     fetch_tasks(int*);
void*   thread_function(void*);

int main(int argc, char** argv) {

    if(argc < 1) {
        perror("Invalid parameters");
        return -1;
    }

    sscanf(argv[1], "%d", &NR_THREADS);
    printf("Value for n: "); scanf("%d", &n); printf("\n");
    printf("Values in array: ");
    for(size_t i = 0; i<n; i++) scanf("%d", &tasks[i]);

    pthread_t thread_ids[NR_THREADS];
    THREAD_DATA threads_data[100000];


    pthread_mutex_t lock_fetch = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t lock_write = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&lock_fetch, NULL);
    pthread_mutex_init(&lock_write, NULL);

    for(size_t i = 0; i<n; i++) {
        threads_data[i].lock_fetch = &lock_fetch;
        threads_data[i].lock_write = &lock_write;
    }

    for(size_t i = 0; i<n; i++) {
        rezultate[i].numar = 0;
        rezultate[i].numar_divizori = 0;
    }

    for(size_t i = 0; i<NR_THREADS; i++) pthread_create(&thread_ids[i], NULL, thread_function, &threads_data[i]);
    for(size_t i = 0; i<NR_THREADS; i++) pthread_join(thread_ids[i], NULL);

    for(size_t i = 0; i<n; i++) {
        printf("%d - %d\n", rezultate[i].numar, rezultate[i].numar_divizori);
    }

    pthread_mutex_destroy(&lock_write);
    pthread_mutex_destroy(&lock_fetch);
    return 0;
}

int numar_divizori(int numar) {
    int result = 0;
    for(size_t divizor = 1; divizor <= numar; divizor++) {
        if(numar % divizor == 0) {
            result++;
        }
    }

    return result;
}

int fetch_tasks(int* tasks) {
   if(CURRENT_INDEX_TASKS == n) return -1;
   return tasks[CURRENT_INDEX_TASKS++];
}

void* thread_function(void* args) {
    THREAD_DATA* current_data = (THREAD_DATA*)args;

    int numar = 0, nr_div = 0;
    while(!FINISHED) {

        pthread_mutex_lock(current_data->lock_fetch);
        numar = fetch_tasks(tasks);
        pthread_mutex_unlock(current_data->lock_fetch);

        nr_div = numar_divizori(numar);
    
        pthread_mutex_lock(current_data->lock_write);
        rezultate[CURRENT_INDEX].numar = numar;
        rezultate[CURRENT_INDEX++].numar_divizori = nr_div;
        pthread_mutex_unlock(current_data->lock_write);

        if(CURRENT_INDEX == n) {
            FINISHED = -1;
            break;
        }
    }
    
    return NULL;
}