#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define ERROR -1

int open_file(char*);
int put_file_in_shm(int, int*);
int modify(char*, char*);

void* SHARED_MEMORY_FILE = NULL;
int FILE_SIZE =  0;

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Invalid input");
        return ERROR;
    }

    int file_descriptor = open_file(argv[1]);
    if(file_descriptor == ERROR) return ERROR;

    if(put_file_in_shm(file_descriptor, &FILE_SIZE) == ERROR) return ERROR;

    argv[2][strlen(argv[2])] = 0;
    argv[3][strlen(argv[3])] = 0;
    modify(argv[2], argv[3]);

    return 0;
}

int open_file(char* path) {

    do {
        int file_descriptor = open(path, O_RDWR);
        if(file_descriptor < 0) {
            perror("Can't open file.");
            break;
        }
        return file_descriptor;
    } while(1);

    return ERROR;
}

int put_file_in_shm(int file_descriptor, int* size) {
    do {

        *size = lseek(file_descriptor, 0, SEEK_END);
        lseek(file_descriptor, 0, SEEK_SET);

        SHARED_MEMORY_FILE = (char*)mmap(0, *size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);
        if(SHARED_MEMORY_FILE == MAP_FAILED) {
            perror("[SHM FILE]: Can't put file in SHM.");
            close(file_descriptor);
            break;
        }

        return 0;
    } while(1);

    return ERROR;
}

int modify(char* first_string, char* second_string) {
    do {
        
        char* sequence = (char*)calloc(strlen(first_string), sizeof(char));

        printf("%s %ld\n%s %ld\n", first_string, strlen(first_string), second_string, strlen(second_string));

        int current_index = 0;
        while(current_index + strlen(first_string) - 1 < FILE_SIZE) {
            memset(sequence, 0, sizeof(sequence));
            memcpy(sequence, SHARED_MEMORY_FILE + current_index, strlen(first_string));

            if(strcmp(sequence, first_string) == 0) {
                printf("MATCH: %s %s\n", sequence, first_string);
                memcpy(SHARED_MEMORY_FILE + current_index, second_string, strlen(sequence));
                current_index += strlen(first_string);           
            }
            else current_index++;
        }

        free(sequence); sequence = NULL;
        return 0;
    } while(1);
    
    return ERROR;
}