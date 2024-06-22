#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#ifndef __PIPE_HELPER__H_
    #define __PIPE_HELPER_H_

    #define PARENT 0
    #define CHILD  1
    #define PIPE_NAME "P2C_FOLDER_DATA"

    #define SUCCESS 0
    #define ERROR  -1
#endif

int open_pipe(char*, unsigned int);
int read_from_pipe(int, unsigned char*, unsigned char*);

int main(int argc, char** argv) {

    int pipe = open_pipe(PIPE_NAME, O_RDONLY);
    if(pipe == ERROR) return ERROR;

    printf("[READ PIPE] SUCCESSFULLY CONNECTED at FILE_DESCRIPTOR %d.\n", pipe);

    printf("%s", "[READ PIPE] DIRECTORY CONTENT INFORMATION:\n");
    do {
        unsigned char data[256];
        unsigned char size = 0;
        int status = read_from_pipe(pipe, data, &size);
        if(status == ERROR) break;
    } while(1);

    close(pipe);
    
    return SUCCESS;
}

int open_pipe(char* path, unsigned int MODE) {
    do {
        int fd = open(path, MODE);
        if(fd == ERROR) {
            perror("[OPEN PIPE] CAN'T OPEN PIPE.");
            break;
        }
        return fd;
    } while(0);

    return ERROR;
}

int read_from_pipe(int pipe, unsigned char* data, unsigned char* size) {
    do {
        int status = read(pipe, size, sizeof(unsigned char));

        if(status != sizeof(unsigned char)) {
            perror("[READ PIPE] CAN'T READ FROM PIPE (1).");
            break;
        }

        status = read(pipe, data, (int)*size * sizeof(unsigned char));
        if(status == 0) { break; }
        if(status != *size * sizeof(unsigned char)) {
            perror("[READ PIPE] CAN'T READ FROM PIPE (2).");
            break;
        } 

        data[(int)*size] = 0;
        printf("\n\n\tNAME SIZE: %d\n\tNAME STRING: %s", (int)*size, data);
        return SUCCESS;
    } while(0);

    return ERROR;
}
