#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define EVALUATE_FAILED(mem) if(mem == MAP_FAILED) { printf("\nMAP_FAILED at line %d\n", __LINE__); perror(""); break; }
#define CHUNK_SIZE 10

int open_file(char*);
int swap_chunk(void*, int, int, size_t, off_t);
int swap_content(int, int);

int main(int argc, char** argv) {

    if(argc < 3) {
        printf("Can't operate with this number of parameters.");
        return -1;
    }

    int  first_file_fd = open_file(argv[1]);
    int second_file_fd = open_file(argv[2]);

    long long first_file_size = lseek(first_file_fd, 0, SEEK_END);
    lseek(first_file_fd, 0, SEEK_SET);

    int LAST_CHUNK_SIZE = 0;
    swap_content(first_file_fd, second_file_fd);

    close(first_file_fd); 
    close(second_file_fd);
    return 0;
}

int open_file(char* path) {
    do {
        int fd = open(path, O_RDWR);
        if(fd < 0) {
            perror("Could not open file.");
            break;
        }

        return fd;
    } while(0);

    exit(-1);
}

int swap_chunk(void* buffer, int first_fd, int second_fd, size_t chunk_size, off_t seek_current) {
    do {
        void* first_file  = (void*)mmap(NULL, chunk_size, PROT_WRITE | PROT_READ, MAP_SHARED, first_fd,  0);
        EVALUATE_FAILED(first_file);

        void* second_file = (void*)mmap(NULL, chunk_size, PROT_WRITE | PROT_READ, MAP_SHARED, second_fd, 0);
        EVALUATE_FAILED(second_file);
        
        memcpy(buffer,                      first_file  + seek_current,  chunk_size);
        memcpy(first_file  + seek_current,  second_file + seek_current,  chunk_size);
        memcpy(second_file + seek_current,  buffer,      chunk_size);

        munmap(first_file, chunk_size);
        munmap(second_file, chunk_size);
        return 1;
    } while(0);

    return -1;
}

int swap_content(int first_fd, int second_fd) {

    do {
        int file_size = lseek(first_fd, 0, SEEK_END);
        lseek(first_fd, 0, SEEK_SET);

        char* buffer = (char*)malloc(CHUNK_SIZE * sizeof(char));
        memset(buffer, 0, CHUNK_SIZE * sizeof(char));

        for(off_t seek_current = 0; seek_current < file_size; seek_current+=CHUNK_SIZE) {
            if(swap_chunk(
                    buffer, first_fd, second_fd, 
                    seek_current + CHUNK_SIZE > file_size ? file_size - seek_current : CHUNK_SIZE, 
                    seek_current
            ) == -1) break;
        }
        
        free(buffer); buffer = NULL;
        return 0;
    } while(0);

    return -1;
}