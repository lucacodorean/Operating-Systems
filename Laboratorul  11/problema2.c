#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

int main(void) {

    sem_t* signal_from_parent, *signal_to_parent;
    signal_from_parent = sem_open("/SIGNAL_FROM_PARENT", O_CREAT, 0644, 1);
    signal_to_parent   = sem_open("/SIGNAL_TO_PARENT",   O_CREAT, 0644, 0);

    int shmFd;
    unsigned int value = 0;
    volatile char *sharedChar = NULL;

    shmFd = shm_open("/l12_myshm", O_CREAT | O_RDWR, 0600);
    if(shmFd < 0) {
        perror("Could not aquire shm");
        return 1;
    }
    ftruncate(shmFd, sizeof(unsigned int));


    volatile unsigned int* address = (volatile unsigned int*)mmap(NULL, sizeof(unsigned int), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if(address == MAP_FAILED) {
        perror("Error while reading in shared memory");
    }

    *(volatile unsigned int*)(address) = value;

    int id = fork();

    if(id == 0) {
        printf("[CHILD]: I was accessed!\n");
        for(int i = 0; i<50; i++) {
            sem_wait(signal_from_parent);
            printf("[CHILD]: %d\n", ++value);
            *(volatile unsigned int*)(address) = value;
            sem_post(signal_to_parent);
        }
    } 
    else {
        printf("[PARENT]: I was accessed!\n");
          for(int i = 0; i<50; i++) {
            sem_post(signal_from_parent);
            printf("[PARENT]: %d\n", ++value);
            *(volatile unsigned int*)(address) = value;
            sem_wait(signal_to_parent);
        }
    }
    
    return 0;
}