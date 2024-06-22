#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#define NOT_ASSIGNED -1;
#define ERROR -1

int N = NOT_ASSIGNED;
char string[11];

typedef struct {
    char first, second;
    pthread_mutex_t* mutex;
} THREAD_STRUCTURE;

void* thread_function(void*);

int init_data(int*, char[11], char**); 

int main(int argc, char** argv) {

    if(argc != 3) {
        printf("Can't operate with these parameters");
        return -1;
    }
    
    int STATUS = init_data(&N, string, argv);
    if(STATUS == ERROR) return ERROR;

    pthread_t thread_ids[N];
    THREAD_STRUCTURE thread_data[N];

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    for(int i = 0; i<N; i++) {
        thread_data[i].first = 'a' + i;
        thread_data[i].second = 'z' - i;
        thread_data[i].mutex = &mutex;
    }

    for(int i = 0; i<N; i++) pthread_create(&thread_ids[i], NULL, thread_function, &thread_data[i]);
    for(int i = 0; i<N; i++) pthread_join(thread_ids[i], NULL);


    printf("STRINGUL PRELUCRAT: %s\n", string);
    pthread_mutex_destroy(&mutex);
    return 0;
}

int init_data(int* N, char string[11], char** argv) {
    int succes = sscanf(argv[1], "%d", N);
    if(succes == 0) {
        sscanf(argv[2], "%d", N);
        if(strlen(argv[1]) < *N) {
            printf("Sir invalid.");
            return ERROR;
        }
        sscanf(argv[1], "%s", string);
    } 
    else {
        if(strlen(argv[2]) < *N) {
            printf("Sir invalid.");
            return ERROR;
        }
        sscanf(argv[2], "%s", string);
    }

    return 0;
}

void* thread_function(void* args) {
    THREAD_STRUCTURE* actual_data = (THREAD_STRUCTURE*)args;

    int contor_first = 0;
    int contor_second = 0;

    for(int i = 0; i<strlen(string); i++) {
        if(string[i] == actual_data->first) contor_first++;
        else if(string[i] == actual_data->second) contor_second ++;
    }

    if(contor_first % 2 == 0) {
        pthread_mutex_lock(actual_data->mutex);
        for(int i = 0; i<strlen(string); i++) {
            if(string[i] == actual_data->first) string[i] = toupper(string[i]);
        }
        pthread_mutex_unlock(actual_data->mutex);
    }

    if(contor_second % 2 == 0) {
        pthread_mutex_lock(actual_data->mutex);
        for(int i = 0; i<strlen(string); i++) {
            if(string[i] == actual_data->second) string[i] = toupper(string[i]);
        }
        pthread_mutex_unlock(actual_data->mutex);
    }

    return NULL;
}