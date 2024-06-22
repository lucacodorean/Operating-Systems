#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR  -1
#define SUCCESS 0
int vowelCount = 0;

typedef struct {
    char     content[100];
    pthread_mutex_t mutex;
} THREAD_STRUCTURE;

void* thread_function(void* param) {
    THREAD_STRUCTURE* actual_parameter = (THREAD_STRUCTURE*) param;

    int temp = 0;
    for(int i = 0; i<strlen(actual_parameter->content); i++) {
        if(strchr("aeiouAEIOU", actual_parameter->content[i])) {
            temp++;
        }
    }

    pthread_mutex_lock(&(actual_parameter->mutex));
    vowelCount += temp;
    pthread_mutex_unlock(&(actual_parameter->mutex)); 
    return NULL;
}

int main(int argc, char* argv[]) {

    if(argc < 2) {
        printf("There were no words sent.");
        return ERROR;
    }


    pthread_t thread_ids[argc-1];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    THREAD_STRUCTURE thread_data[argc-1];
    for(int i = 1; i<argc; i++) {
        printf("New content: %s\n", argv[i]);
        strcpy(thread_data[i-1].content, argv[i]);
        thread_data[i-1].mutex = mutex;
    }

    for(int i = 0; i<argc-1; i++) pthread_create(&thread_ids[i], NULL, thread_function, &thread_data[i]);
    for(int i = 0; i<argc-1; i++) pthread_join(thread_ids[i], NULL);
    pthread_mutex_destroy(&mutex);

    printf("Se gasesc %d vocale in cele %d cuvinte.\n", vowelCount, argc-1);
    return SUCCESS;
}