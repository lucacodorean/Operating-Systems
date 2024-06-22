#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct {
    int left, right;
    int step;
    pthread_mutex_t* WRITING_MUTEX;
} THREAD_STRUCTURE;

int current_index = 0, N, M;
unsigned int* result = NULL;

int is_perfect(unsigned int);
void* thread_function(void*);

int main(int argc, char** argv) {

    if(argc < 3) {
        return -1;
    }

    sscanf(argv[1], "%d", &N);
    sscanf(argv[2], "%d", &M);

    result = (unsigned int*)malloc(N * sizeof(unsigned int));
    if(result == NULL) {
        perror("Can't allocate such memory.");
        return -1;
    }
    
    memset(result, 0, N * sizeof(unsigned int));

    pthread_mutex_t WRITING_MUTEX = PTHREAD_MUTEX_INITIALIZER;

    THREAD_STRUCTURE thread_information[M];
    pthread_t threads[M];

    int loc = 1;
    for(int i = 0; i<M; i++) {
        thread_information[i].WRITING_MUTEX = &WRITING_MUTEX;
        thread_information[i].left  = loc++;
        thread_information[i].right = N;
        thread_information[i].step  = M;
    }

    for(int i = 0; i<M; i++) pthread_create(&threads[i], NULL, thread_function, &thread_information[i]);
    for(int i = 0; i<M; i++) pthread_join(threads[i], NULL);


    for(int i = 0; i<current_index; i++) printf("%d ", result[i]);
    pthread_mutex_destroy(&WRITING_MUTEX);
    free(result); result = NULL;
    return 0;
}

int is_perfect(unsigned int number) {
    int sum = 0;

    for(int d = 1; d<number; d++) {
        if(number % d == 0) sum += d;
    }

    return number == sum;
}

void* thread_function(void* args) {
    THREAD_STRUCTURE* argument = (THREAD_STRUCTURE*)args;

    int NUMBERS[N]; int l = 0;

    for(int actual_number = argument->left; actual_number<argument->right; actual_number+=argument->step) {
        if(is_perfect(actual_number) == 1) {
            NUMBERS[l++] = actual_number;
        }
    }

    pthread_mutex_lock(argument->WRITING_MUTEX);
    for(int i = 0; i<l; i++) result[current_index++] = NUMBERS[i];
    pthread_mutex_unlock(argument->WRITING_MUTEX);
}