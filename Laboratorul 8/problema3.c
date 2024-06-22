#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


#ifndef __CONSTANTS__H__
    #define NUMBER_OF_THREADS 3
    typedef struct {
        pthread_mutex_t* lock;

    } THREAD_DATA;
#endif


int file_status = 0;
int   open_once(void*);
void* thread_function(void*);

int main(void) {

    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&lock, NULL);

    pthread_t ids[NUMBER_OF_THREADS];
    THREAD_DATA threads_data[NUMBER_OF_THREADS];

    for(size_t i = 0; i<NUMBER_OF_THREADS; i++) threads_data[i].lock = &lock;
    for(size_t i = 0; i<NUMBER_OF_THREADS; i++) pthread_create(&ids[i], NULL, thread_function, &threads_data[i]);
    for(size_t i = 0; i<NUMBER_OF_THREADS; i++) pthread_join(ids[i], NULL);
    pthread_mutex_destroy(&lock);
    return 0;
}

/// @brief Deschide fisierul si verifica daca fisierul e deja deschis.
/// @brief Ne folosim de open, care returneaza statusul fisierului, status de care ne folosim ca sa stim ca putem sa citim, exact un byte.
/// @brief Open returneaza 1 daca s-a deschis fisierul, iar acea valoarea 1, poate sa fie folosita si ca octet.
/// @param args 
/// @return 
int open_once(void* args){
    THREAD_DATA* current_value = (THREAD_DATA*)args;

    pthread_mutex_lock(current_value->lock);
    if(file_status == 0) file_status = open("temp.in", O_RDONLY);
    pthread_mutex_unlock(current_value->lock);
    return file_status;
}

void* thread_function(void* args) {
    THREAD_DATA* actual_value = (THREAD_DATA*)args;

    int fd = open_once(actual_value);

    int value_to_be_read = 0;
    pthread_mutex_lock(actual_value->lock);
    int succes = read(fd, &value_to_be_read, sizeof(int)/4);
    pthread_mutex_unlock(actual_value->lock);
    
    if(succes == sizeof(int)/4) printf("%c", (char)value_to_be_read);
    else printf("Nu se poate realiza citirea.");
    
    return NULL;
}