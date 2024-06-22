#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int start, stop, step;
    pthread_mutex_t* mutex;
} THREAD_STRUCTURE;

int result = 0, result_size = 0;
int arr[1000], perfect_numbers[1000];

int is_perfect(int number) {
    int sum = 0;
    for(int i = 1; i<number; i++) {
        if(number % i == 0) sum += i;
    }

    return number == sum;
}

void* thread_function(void* args) {
    THREAD_STRUCTURE* arg = (THREAD_STRUCTURE*)args;

    int temp = 0;
    int buffer[1000] = {0};
    int current_size = 0;

    for(int i = arg->start; i<arg->stop; i+=arg->step) {
        if(is_perfect(arr[i])== 1) {
            temp+= arr[i];
            buffer[current_size++] = arr[i];
        }
    }

    pthread_mutex_lock(arg->mutex);
    result += temp;
    for(int i = 0; i<current_size; i++) perfect_numbers[result_size++] = buffer[i];
    pthread_mutex_unlock(arg->mutex);
    return NULL;
}


int main(void) {

    int N, M;
    printf("N= "); scanf("%d", &N);
    printf("M= "); scanf("%d", &M);
    
    if(N > 1000 || N > 16) {
        printf("Invalid data.\n");
        return -1;
    }


    for(int i = 0; i<N; i++) scanf("%d", &arr[i]);

    pthread_t threads[M];
    THREAD_STRUCTURE therad_data[M];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    int position = 0;
    for(int i = 0; i<M; i++) {
        therad_data[i].step  = M;
        therad_data[i].stop  = N;
        therad_data[i].start = position++; 
        therad_data[i].mutex = &mutex;
    }


    for(int i = 0; i<M; i++) pthread_create(&threads[i], NULL, thread_function, &therad_data[i]);
    for(int i = 0; i<M; i++) pthread_join(threads[i], NULL);

    printf("Suma numerelor perfecte din array este: %d\nNumerele sunt: ", result);
    for(int i = 0; i<result_size; i++) printf("%d ", perfect_numbers[i]);
    return 0;
}