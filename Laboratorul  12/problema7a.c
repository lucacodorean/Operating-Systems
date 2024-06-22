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

int close_pipe(char*, int);
int create_pipe(char*); 
int open_pipe(char*, unsigned int);
int write_to_pipe(int, unsigned char*, unsigned char);
int get_directory_files(char*, int);

int main(int argc, char** argv) {

    if(argc != 2) {
        printf("Can't operate with these parameters.");
        return ERROR;
    }

    int status = create_pipe(PIPE_NAME);
    if(status == ERROR) return ERROR;

    int pipe = open_pipe(PIPE_NAME, O_WRONLY);
    if(pipe == ERROR) return ERROR;

    printf("[WRITE PIPE] SUCCESSFULLY CONNECTED at FILE_DESCRIPTOR %d.\n", pipe);

    status = get_directory_files(argv[1], pipe);
    if(status == ERROR) return ERROR;

    close_pipe(PIPE_NAME, pipe);
    
    return SUCCESS;
}

int create_pipe(char* path) {
    do {
        int status = mkfifo(path, 0644);
        if(status != SUCCESS) {
            perror("[CREATE PIPE] CAN'T CREATE PIPE.");
            break;
        }
        return SUCCESS;
    } while(0);

    return ERROR;
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

int write_to_pipe(int pipe, unsigned char* data, unsigned char size) {
    do {
        printf("\n[WRITE PIPE] SE SCRIE IN PIPE %d %s!", (int)size, data);
        int status = write(pipe, &size, sizeof(unsigned char));
        if(status != sizeof(unsigned char)) {
            perror("[WRITE PIPE] CAN'T WRITE TO PIPE (1).");
            break;
        }

        status = write(pipe, data, size * sizeof(unsigned char));
        if(status != size * sizeof(unsigned char)) {
            perror("[WRITE PIPE] CAN'T WRITE TO PIPE (2).");
            break;
        }

        return SUCCESS;
    } while(0);

    return ERROR;
}

int get_directory_files(char* path, int pipe) {
    do {
        DIR* current_dir = opendir(path);
        if(current_dir == NULL) {
            perror("Can't open the directory.");
            break;
        }

        struct dirent* current_file = readdir(current_dir);
        while(current_file != NULL) {                                          
            if(strcmp(current_file->d_name, ".") && strcmp(current_file->d_name, "..")) {       
                struct stat current_directory_stats;
                char filePath[512];
                snprintf(filePath, 512, "%s/%s", path, current_file->d_name);

                lstat(filePath, &current_directory_stats);
                if(!S_ISDIR(current_directory_stats.st_mode)) write_to_pipe(pipe, filePath, strlen(filePath));
            }

            current_file = readdir(current_dir); 
        }

        write_to_pipe(pipe, 0x00, 0);
        return SUCCESS;
    } while(0);

    return ERROR;
}

int close_pipe(char* path, int fd) {
    unlink(path);
    close(fd);
}