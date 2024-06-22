#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* threadFn(void*);


int main(int argc, char** argv) {

    pthread_t pid = -1;

    if(pthread_create(&pid, NULL, threadFn, NULL) != 0) {
        perror("Error at line 11");
        return -1;
    }


    printf("This is the main thread!\n");
    pthread_join(pid, NULL);
    return 0;
}

void* threadFn(void* parameter) {
    printf("This is the called thread!\n");
    return NULL;
}