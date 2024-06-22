#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#define TRUE 1
#define FALSE 0
#define C2P "C2P"
#define P2C "P2C"
#define ERROR 1
#define SUCCESS 0
#define CHKBRK(VALUE) { if(VALUE == 1) return 1; }

int create_pipe(char*);
int open_pipe(char*, mode_t);
int write_to_pipe(int, char*, unsigned int);
int read_from_pipe(int, char*, unsigned int*);
int parse_command(char*, char[256], char[256]);

int execute_first_task(char*, struct dirent*[], int*);
int execute_second_task(char*);

int main(void) {

    int READ_PIPE  = open_pipe(C2P, O_RDONLY);  CHKBRK(READ_PIPE);
    int WRITE_PIPE = open_pipe(P2C, O_WRONLY);  CHKBRK(WRITE_PIPE);


    char command[256], parameter[256];
    unsigned int size_of_command = 0;
    unsigned int size_of_parameter = 0;
    while(TRUE) {
        memset(command, 0, 256);
        memset(parameter, 0, 256);

        CHKBRK(read_from_pipe(READ_PIPE, command, &size_of_command));
        printf("[SERVER]: Command received: %s\n", command);

        CHKBRK(read_from_pipe(READ_PIPE, parameter, &size_of_parameter));
        printf("[SERVER]: Parameter received: %s\n", parameter);

        if(strcmp(command, "fisier") == 0)  {
            int size = 0;
            struct dirent* data[256];
            int status = execute_first_task(parameter, data, &size);

            if(status != ERROR && size != 0) {
                write(WRITE_PIPE, &size, sizeof(int));
                for(int i = 0; i<size; i++) CHKBRK(write_to_pipe(WRITE_PIPE, data[i]->d_name, strlen(data[i]->d_name)));
            }
            continue;
        }
        else if(strcmp(command, "rev") == 0) {
            char temp[256];
            strcpy(temp, parameter);
            strcat(temp, "_inversat");
            if(execute_second_task(parameter) == SUCCESS) write_to_pipe(WRITE_PIPE, temp, strlen(temp));
        }
        else  printf("[SERVER]: Invalid command.\n");
    }

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

        printf("[SERVER]: S-a scris in pipe %d %s\n", size, string);
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

int execute_first_task(char* text, struct dirent* files[], int* size) {
    DIR* current_directory = opendir(".");

    if(current_directory == NULL) return ERROR;

    struct dirent* current_file = readdir(current_directory);
    while(current_file != NULL) {
        if(strstr(current_file->d_name, text) != NULL) {
            files[(*size)++] = current_file;
        }
        current_file = readdir(current_directory);
    }

    closedir(current_directory);
    return SUCCESS;
}

int execute_second_task(char* path) {
    do {
        char new_path[256];
        strcpy(new_path, path);
        strcat(new_path, "_inversat");

        int new_fd = creat(new_path, O_CREAT | S_IWUSR | S_IRUSR | S_IROTH | S_IRGRP);
        if(new_fd < 0) break;

        int fd = open(path, O_RDONLY);
        if(new_fd < 0) break;

        int size = lseek(fd, 0, SEEK_END), ok = SUCCESS;
        lseek(fd, 0, SEEK_SET);
        while(size > 0) {
            lseek(fd, size-1, SEEK_SET);
            char temp;
            int success = read(fd, &temp, sizeof(char));
            if(success != sizeof(char)) {
                ok = ERROR;
                printf("Can't read from file at path %s", path);
                break;
            }

            success = write(new_fd, &temp, sizeof(char));
            if(success != sizeof(char)) {
                ok = ERROR;
                printf("Can't write in file at path %s", new_path);
                break;
            }
            size--;
        }

        CHKBRK(ok);
        close(fd);
        close(new_fd);
        return SUCCESS;
    } while(TRUE);

    return ERROR;
}