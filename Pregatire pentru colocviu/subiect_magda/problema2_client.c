#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define C2P "C2P"
#define P2C "P2C"
#define ERROR 1
#define SUCCESS 0
#define CHKBRK(VALUE) { if(VALUE == 1) return 1; }
#define STRING_AND_SIZE(X) X, strlen(X)

int create_pipe(char*);
int open_pipe(char*, mode_t);
int write_to_pipe(int, char*, unsigned int);
int read_from_pipe(int, char*, unsigned int*);

int main(void) {

    char command[256], parameter[256];
    
    int READ_PIPE  = create_pipe(P2C); CHKBRK(READ_PIPE);
    int WRITE_PIPE = create_pipe(C2P); CHKBRK(WRITE_PIPE);

    WRITE_PIPE = open_pipe(C2P, O_WRONLY); CHKBRK(WRITE_PIPE);
    READ_PIPE  = open_pipe(P2C, O_RDONLY); CHKBRK(READ_PIPE);


    while(TRUE) {
        memset(command, 0, 256);
        memset(parameter, 0, 256);

        printf("\n[CLIENT]: Introduceti comanda pe care doriti sa o executati: ");
        scanf("%s %s", command, parameter);
        if(strcmp(command, "END") == 0) break;

        CHKBRK(write_to_pipe(WRITE_PIPE, STRING_AND_SIZE(command)));
        CHKBRK(write_to_pipe(WRITE_PIPE, STRING_AND_SIZE(parameter)));
        printf("[CLIENT]: Command and parameter sent: %s %s\n", command, parameter);

        if(strcmp(command, "fisier") == 0) {
            int size = 0, size_of_filename = 0;
            read(READ_PIPE, &size, sizeof(int));

            char filename[256];
            for(int i = 0; i<size; i++) {
                memset(filename, 0, 256); 
                CHKBRK(read_from_pipe(READ_PIPE, filename, &size_of_filename));
                printf("[CLIENT]: Received from server: %s\n", filename);
            }
        }
        else if(strcmp(command, "rev") == 0) {
            char filename[256];
            int size_of_filename = 0;
            memset(filename, 0, 256); 
            CHKBRK(read_from_pipe(READ_PIPE, filename, &size_of_filename));
            printf("[CLIENT]: Received from server: %s\n", filename);
        }

        else continue;
    }



    unlink(C2P);
    unlink(P2C);
    close(READ_PIPE);
    close(WRITE_PIPE);
    return SUCCESS;
}

int create_pipe(char* path) {
    do{
        int success = -1;
        if((success = mkfifo(path, 0600)) < 0) {
            perror("[CREATE PIPE] Can't create the pipe.\n");
            break;
        }

        return SUCCESS;
    } while(FALSE);

    return ERROR;
}

int open_pipe(char* path, mode_t mode) {
    do{
        int fd = -1;
        if((fd = open(path, mode)) < 0) {
            perror("[OPEN PIPE] Can't open the pipe.\n");
            break;
        }

        return fd;
    } while(FALSE);

    return ERROR;
}

int write_to_pipe(int pipe, char* string, unsigned int size) {

   do{
        int status = write(pipe, &size, sizeof(unsigned int));
        if(status != sizeof(unsigned int)) {
            perror("[WRITE TO PIPE 1] Can't write to pipe.\n");
            break;
        }

        status = write(pipe, string, strlen(string) * sizeof(char));
        if(status != strlen(string) * sizeof(char)) {
            perror("[WRITE TO PIPE 2] Can't write to pipe.\n");
            break;
        }

        return SUCCESS;
    } while(FALSE);

    return ERROR;
}

int read_from_pipe(int pipe, char* string, unsigned int* size) {

   do{
        int status = read(pipe, size, sizeof(unsigned int));
        if(status != sizeof(unsigned int)) {
            perror("[READ FROM PIPE 1] Can't read to pipe.\n");
            break;
        }

        status = read(pipe, string, (*size) * sizeof(char));
        if(status != (*size) * sizeof(char)) {
            perror("[READ TO PIPE 2] Can't read to pipe.\n");
            break;
        }

        return SUCCESS;
    } while(FALSE);

    return ERROR;
}