#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define WRITE_PIPE_NAME "C2P"
#define READ_PIPE_NAME  "P2C"

#define READ  0 
#define WRITE 1
#define SUCCESS 0
#define ERROR 1

int  create_pipe(char*);
int  open_pipe(char*, unsigned int);
int  write_into_pipe(int, char[256], unsigned int);
int  read_from_pipe(int, char[256], unsigned int*);

int main(int argc, char** argv) {
    if(argc != 3) {
        printf("Can't operate with these parameters");
        return -1;
    }

    
    int READ_PIPE = create_pipe(READ_PIPE_NAME);
    if(READ_PIPE == ERROR) return ERROR;

    int PIPE = create_pipe(WRITE_PIPE_NAME);
    if(PIPE == ERROR) return ERROR;
    
    PIPE = open_pipe(WRITE_PIPE_NAME, O_WRONLY);
    if(PIPE == ERROR) return ERROR;

    printf("PIPES CONNECTED;\n");

    char path[256];  memset(path, 0, 256);  strcpy(path, argv[1]);
    char text[256];  memset(text, 0, 256);  strcpy(text, argv[2]);

    int sucess = write_into_pipe(PIPE, path, strlen(path)); 
    if(sucess == ERROR) return ERROR;

    sucess = write_into_pipe(PIPE, text, strlen(text)); 
    if(sucess == ERROR) return ERROR;


    READ_PIPE = open_pipe(READ_PIPE_NAME, O_RDONLY);
    if(READ_PIPE == ERROR) return ERROR;
    char output[256]; 
    unsigned int output_size = 0;
    sucess = read_from_pipe(READ_PIPE, output, &output_size);
    if(sucess == ERROR) return ERROR;

    int status = -1;
    sucess = read(READ_PIPE, &status, sizeof(int));
    if(sucess == ERROR) return ERROR;

    printf("\nRECEIVED FROM SERVER:\n\t");
    printf("filename: %s\n\t", output);
    printf("status: %d\n", status);


    close(PIPE); close(READ_PIPE);
    unlink(WRITE_PIPE_NAME); unlink(READ_PIPE_NAME);
    return SUCCESS;
}

int create_pipe(char* path) {
    do {
        int status = mkfifo(path, 0600);
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

int write_into_pipe(int fd, char string[256], unsigned int size) {

    do {
        int status = write(fd, &size, sizeof(unsigned int));
        if(status != sizeof(size)) {
            perror("[WRITE TO PIPE 1] Can't write the size");
            break;
        }

        status = write(fd, string, strlen(string) * sizeof(char));
        if(status != strlen(string) * sizeof(char)) {
            perror("[WRITE TO PIPE 2] Can't write the string.");
            break;
        }

        printf("S-a scris in PIPE %d %s.\n", size, string);
        return SUCCESS;

    } while(1);
    return ERROR;
}

int read_from_pipe(int fd, char* string, unsigned int* size) {
    do {
        int status = read(fd, size, sizeof(unsigned int));
        if(status != sizeof(unsigned int)) {
            printf("%d", status);
            perror("[READ FROM PIPE 1] Can't read from pipe.");
            break;
        } 

        status = read(fd, string, (*size));
        if(status != *size * sizeof(char)) {
            perror("[READ FROM PIPE 2] Can't read from pipe.");
            break;
        }
        printf("S-a citit %d %s", *size, string);
        return SUCCESS;
    } while(1);

    return ERROR;
}
