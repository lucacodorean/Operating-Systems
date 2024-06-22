#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

long long Func(int, int);

#define CHILD       0
#define ERROR       -1
#define SUCCESS     1
#define READ        0
#define WRITE       1
#define MEMORY_NAME "SHAREDMEM"

void* MEMORY_ADDRESS = NULL;

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

        ftruncate(sfd, 8 * sizeof(long long));
        MEMORY_ADDRESS = (char*)mmap(0, 10 * sizeof(long long), PROT_READ | PROT_WRITE, MAP_SHARED, sfd, 0);

        if(MEMORY_ADDRESS == MAP_FAILED) {
            printf("Can't allocate the mmap.\n");
            break;
        }
        return EXIT_SUCCESS;
    } while(1);
    return ERROR;
}

int write_in_memory(long long value, int offset) {
    do {
        *(volatile long long*)(MEMORY_ADDRESS + offset) = value;
        return SUCCESS;
    } while(1);

    return ERROR;
}

int read_from_memory(long long* value, int offset) {
    do {
        *value = (volatile long long*)(MEMORY_ADDRESS + offset);
        return SUCCESS;
    } while(1);

    return ERROR;
}


int main(int argc, char** argv) {

    if(argc != 2) {
        printf("Invalid parameters.");
        return ERROR;
    }

    int N = 0;

    sscanf(argv[1], "%d", &N);

    if(N%2 != 0) {
        printf("Invalid parameters.");
        return ERROR;
    }

    for(int i = 0; i<N; i+=2) {
        pid_t process1 = fork();
        if(process1 == CHILD) {
            write_in_memory(Func(0,5), 0);
            write_in_memory(Func(2, 856),      sizeof(long long));
            write_in_memory(Func(2, 8224), 2 * sizeof(long long));
            write_in_memory(Func(4, 0),    3 * sizeof(long long));
            write_in_memory(Func(4, 1),    4 * sizeof(long long));
            exit(SUCCESS);
        }

        pid_t process2 = fork();
        if(process2 == CHILD) {
            write_in_memory(Func(0,5),      5 *  sizeof(long long));
            write_in_memory(Func(2, 856),   6 *  sizeof(long long));
            write_in_memory(Func(2, 8224),  7 *  sizeof(long long));
            write_in_memory(Func(4, 0),     8 *  sizeof(long long));
            write_in_memory(Func(4, 1),     9 *  sizeof(long long));
            exit(SUCCESS);
        }


        waitpid(process1, NULL, 0);
        waitpid(process2, NULL, 0);

        for(int j = 0; j<10; j++) {
            unsigned long long value = 0;
            read_from_memory(&value, j * sizeof(long long));

            printf("PROC[%d] Func(a,b) = %lld\n", j < 5 ? i : i+1, value);
        }

        printf("\n\n");
    }

    printf("\nCompilat cu succes.");
    return 0;
}


long long Func(int x, int y) {
    if(x == 0) return y + 1;
    if(y == 0) return Func(x-1, 1);
    
    return Func(x-1, Func(x, y-1));
}
