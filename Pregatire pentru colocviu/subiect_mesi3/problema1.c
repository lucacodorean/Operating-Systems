#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#define ERROR -1
#define MEMORY_NAME "/CITIES_MEM"

void* MEMORY_ADDRESS = NULL;

typedef struct {
    int city_id;
    char* cities;
    pthread_mutex_t* mutex;
} THREAD_STRUCTURE;

int create_shm();
int open_file_in_memory(int, char*);
int init_shared_memory(int, char*);
int put_data_in_shm(char*);

void* thread_function(void*);

int maximum_neighbors = 0, index_of_city = 0;

int main(int argc, char** argv) {

    if(argc != 2) {
        printf("Invalid number of arguments.");
        return -1;
    }

    char path[256]; memset(path, 0, 256);
    strcpy(path, argv[1]);

    int sfd = create_shm();
    int success = init_shared_memory(sfd, path);
    if(success == ERROR) return ERROR;

    success = put_data_in_shm(path);
    if(success == ERROR) return ERROR;

    unsigned char size = -1;
    memcpy(&size, MEMORY_ADDRESS, sizeof(char));

    int CITIES_NUMBER = size - '0';
    MEMORY_ADDRESS = MEMORY_ADDRESS + 2;

    pthread_t threads[CITIES_NUMBER];
    THREAD_STRUCTURE thread_data[CITIES_NUMBER];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    unsigned int OFFSET = CITIES_NUMBER * sizeof(char);

    for(int i = 0; i<CITIES_NUMBER; i++) {
        thread_data[i].city_id = i;
        thread_data[i].mutex = &mutex;

        thread_data[i].cities = (char*)malloc(CITIES_NUMBER * sizeof(char));

        if(thread_data[i].cities == NULL) {
            printf("Can't allocat memory for city.");
            break;
        }

        memset(thread_data[i].cities, 0, CITIES_NUMBER * sizeof(char));
        memcpy(thread_data[i].cities, MEMORY_ADDRESS, CITIES_NUMBER * sizeof(char));
        MEMORY_ADDRESS = MEMORY_ADDRESS + OFFSET + 1;
    }

    for(int i = 0; i<CITIES_NUMBER; i++) pthread_create(&threads[i], 0, thread_function, &thread_data[i]);
    for(int i = 0; i<CITIES_NUMBER; i++) pthread_join(threads[i], NULL);

    printf("Orasul %d are cei mai multi vecini %d", index_of_city, maximum_neighbors);

    return 0;
}

int create_shm() {
    do {
        int shmFD = shm_open(MEMORY_NAME, O_CREAT | O_RDWR, 0600);
        if(shmFD < 0) {
            printf("Couldn't create the shared memory");
            break;
        }
        return shmFD;
    } while(1);
    return ERROR;
}

int init_shared_memory(int sfd, char* path) {
    do {

        int file_fd = open(path, O_RDONLY);
        if(file_fd < 0) {
            printf("Can't open the file.\n");
            break;
        }

        int size = lseek(file_fd, 0, SEEK_END);
        lseek(file_fd, 0, SEEK_SET);

        ftruncate(sfd, size);
        MEMORY_ADDRESS = (char*)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, sfd, 0);

        if(MEMORY_ADDRESS == MAP_FAILED) {
            printf("Can't allocate the mmap.\n");
            break;
        }
        return EXIT_SUCCESS;
    } while(1);
    return ERROR;
}

int put_data_in_shm(char* path) {
    do {
        int file_fd = open(path, O_RDONLY);
        if(file_fd < 0) {
            printf("Can't open the file.\n");
            break;
        }

        int size = lseek(file_fd, 0, SEEK_END);
        lseek(file_fd, 0, SEEK_SET);

        char* address = (char*)mmap(0, size, PROT_READ, MAP_SHARED, file_fd, 0);
        if(address == MAP_FAILED) {
            printf("Can' allocate address.\n");
            break;
        }

        memset(MEMORY_ADDRESS, 0, size);
        memcpy(MEMORY_ADDRESS, address, size);
        return EXIT_SUCCESS;
    } while(1);
    return ERROR;
}

void* thread_function(void* args) {
    THREAD_STRUCTURE* arg = (THREAD_STRUCTURE*) args;

    int number_of_neighbors = 0;
    for(int i = 0; i<strlen(arg->cities); i++) number_of_neighbors += (arg->cities[i] == '1');

    pthread_mutex_lock(arg->mutex);
    if(number_of_neighbors > maximum_neighbors) {
        index_of_city = arg->city_id;
        maximum_neighbors = number_of_neighbors;
    }
    pthread_mutex_unlock(arg->mutex);
    return NULL;
}