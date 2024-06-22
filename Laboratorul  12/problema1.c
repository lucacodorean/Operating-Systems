#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define ERROR    -1
#define SUCCESS   0
#define FALSE     0
#define READ      0
#define WRITE     1

int create_nameless_pipe();
int write_into_pipe(int, char*);
int read_from_pipe(int);

int main(int argc, char** argv) {
    int fd[2];

    int success = create_nameless_pipe(fd);

    if(fork() != 0) {
        
        close(fd[READ]);

        int opened_file = open(argv[1], O_RDONLY);
        if(opened_file < 0) {
            printf("%s", "FILE ERROR!");
            return ERROR;
        }
        printf("FILE %s OPENED.\n", argv[1]);

        unsigned char letter;
        while(letter != EOF) {

            int success = read(opened_file, &letter, sizeof(letter));
            if(success != 1) { break;}

            if(isdigit(letter)) write_into_pipe(fd[WRITE], &letter);
        }

        close(opened_file);
    }

    else {
        close(fd[WRITE]);
        do {} while(read_from_pipe(fd[READ]) != EOF);
    }


    return SUCCESS;
}

int create_nameless_pipe(int fd[2]) {
    do {
        int success = pipe(fd);
        if(success == ERROR) {
            perror("[CREATE NAMELESS PIPE ERROR]");
            break;
        }

        printf("PIPE HAS BEEN CREATED.\n");
        return SUCCESS;
    } while(FALSE);

    return ERROR;
}

int write_into_pipe(int pipe, char* character) {
    do {
        int success = write(pipe, character, sizeof(char));
        if(success == ERROR) {
            perror("[WRITE TO PIPE ERROR]");
            break;
        }
        printf("[PIPE WRITE] %c\n", *character);
        return SUCCESS;
    } while(FALSE);

    return ERROR;
}

int read_from_pipe(int pipe) {
    do {
        char character;
        int success = read(pipe, &character, sizeof(char));
        
        if(success != sizeof(char)) {
            perror("[CHILD READ FROM PIPE ERROR]");
            break;
        }
        printf("[PIPE READ] %c\n", character);
        return SUCCESS;
    } while(FALSE);
    return ERROR;
}

