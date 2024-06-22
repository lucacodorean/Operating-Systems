#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>

#define WRITE_PIPE_NAME "C2P"
#define READ_PIPE_NAME  "P2C"

#define READ  0 
#define WRITE 1
#define SUCCESS 0
#define ERROR 1

typedef struct {
    char path[256];
    int step;
    int lower_bound; 
    int upper_bound;
    pthread_mutex_t* mutex;
} THREAD_STRUCTURE;

int  create_pipe(char*);
int  open_pipe(char*, unsigned int);
int  read_from_pipe(int, char*, unsigned int*);
int  write_into_pipe(int, char*, unsigned int);
void* thread_function(void* args);

int count = 0;
int main(int argc, char** argv) {


    int READ_PIPE = open_pipe(WRITE_PIPE_NAME, O_RDONLY);
    if(READ_PIPE == ERROR) return ERROR;

    unsigned int size = 0;
    char path[256]; memset(path, 0, 256); 

    int status = read_from_pipe(READ_PIPE, path, &size);
    if(status == ERROR) return ERROR;

    unsigned int data_size = 0;
    char string[256]; memset(string, 0, 256); 

    status = read_from_pipe(READ_PIPE, string, &data_size);
    if(status == ERROR) return ERROR;

    struct stat FILE_DATA;
    stat(path, &FILE_DATA);

    int fd = -1;

    pthread_t threads[4];
    THREAD_STRUCTURE thread_data[4];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int loc = 0;
    for(int i = 0; i<4; i++) {
        strcpy(thread_data[i].path, path);
        thread_data[i].upper_bound = FILE_DATA.st_size;
        thread_data[i].lower_bound = loc++;
        thread_data[i].mutex = &mutex;
        thread_data[i].step = 4;
    }

    for(int i = 0; i<4; i++) pthread_create(&threads[i], NULL, thread_function, &thread_data[i]);
    for(int i = 0; i<4; i++) pthread_join(threads[i], NULL);

    if(S_ISREG(FILE_DATA.st_mode)) {
        char c;
        fd = open(path, O_RDONLY);
        if(fd < 0) {
            close(fd);
            close(READ_PIPE);
            return ERROR;
        }

        char buffer[FILE_DATA.st_size];
        memset(buffer, 0, FILE_DATA.st_size);
        read(fd, buffer, FILE_DATA.st_size);
        close(fd);

        fd = open(path, O_WRONLY);
        if(fd < 0) {
            close(fd);
            close(READ_PIPE);
            return ERROR;
        }

        lseek(fd, 0, SEEK_SET);
        for(int i = 0; i<strlen(buffer); i++) {
            if(buffer[i] == ' ') {
                write(fd, string, data_size);
            } else write(fd, &buffer[i], sizeof(char));
        }

        close(fd);
    }
    

    int created_file_fd = creat("metadata.txt", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    write(created_file_fd, &FILE_DATA.st_size, sizeof(off_t));

    unsigned char spacer = 0xA;
    status = write(created_file_fd, &spacer, sizeof(unsigned char));
    if(status != sizeof(unsigned char)) return ERROR;

    status = write(created_file_fd, &count, sizeof(int));
    if(status != sizeof(int)) {
        printf("%d", status);
        return ERROR;
    }
    close(created_file_fd);

    int WRITE_PIPE = open_pipe(READ_PIPE_NAME, O_WRONLY);
    if(WRITE_PIPE == ERROR) return ERROR;

    status = write_into_pipe(WRITE_PIPE, "metadata.txt", strlen("metadata.txt"));
    if(status == ERROR) return ERROR;

    int send_value = (status == SUCCESS ? 1 : 0);

    status = write(WRITE_PIPE, &send_value, sizeof(int));
    if(status != sizeof(int)) return ERROR;
    

    close(WRITE_PIPE);
    unlink(READ_PIPE_NAME);
    printf("OK!");
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
    } while(1);

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
    } while(1);

    return ERROR;
}

int read_from_pipe(int fd, char* string, unsigned int* size) {
    do {
        int status = read(fd, size, sizeof(unsigned int));
        if(status != sizeof(unsigned int)) {
            perror("[READ FROM PIPE 1] Can't read from pipe.");
            break;
        } 

        status = read(fd, string, (*size));
        if(status != *size * sizeof(char)) {
            perror("[READ FROM PIPE 2] Can't read from pipe.");
            break;
        }

        return SUCCESS;
    } while(1);

    return ERROR;
}

int write_into_pipe(int fd, char* string, unsigned int size) {
    do {
        int status = write(fd, &size, sizeof(unsigned int));
        if(status != sizeof(unsigned int)) {
            perror("[WRITE TO PIPE 1] Can't write the size");
            break;
        }

        status = write(fd, string, strlen(string) * sizeof(char));
        if(status != strlen(string) * sizeof(char)) {
            perror("[WRITE TO PIPE 2] Can't write the string.");
            break;
        }

        printf("S-a scris in pipe-ul %d: %d %s\n", fd, size, string);
 
        return SUCCESS;
    } while(1);
    return ERROR;
}

void* thread_function(void* args) {
    THREAD_STRUCTURE* arg = (THREAD_STRUCTURE*)args;

    int internal_counter = 0;

    int fd = open(arg->path, O_RDONLY);
    for(int i = arg->lower_bound; i<arg->upper_bound; i+=arg->step) {
        char temp = 0;
        int status = read(fd, &temp, 1);
        internal_counter += (temp == ' ');
    }
    close(fd);

    pthread_mutex_lock(arg->mutex);
    count += internal_counter; 
    pthread_mutex_unlock(arg->mutex);
    return NULL;
}