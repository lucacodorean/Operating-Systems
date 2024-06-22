#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <semaphore.h>

#define ERROR  -1
#define SUCCESS 0
#define CHILD   0
#define READ    0
#define WRITE   1

int open_file(char*);
int open_pipe(int*);
int write_in_pipe(int, char*,  unsigned char);
int read_from_pipe(int, char*, unsigned char*);

int dispatch_line(int, int, char*, unsigned char, sem_t*, sem_t*);

int P2B[2] = {0}, P2E[2] = {0};

int main(int argc, char** argv) {

    if(argc != 2) {
        printf("Invalid input.");
        return -1;
    }

    sem_t basic_operators_sem;
    sem_t extended_operators_sem;

    sem_init(&basic_operators_sem,      1, 1);
    sem_init(&extended_operators_sem,   1, 1); 

    int success = open_pipe(P2B);
    if(success == ERROR) return ERROR;

    success = open_pipe(P2E);
    if(success == ERROR) return ERROR;

    pid_t basic_operators, extended_operators;

    basic_operators = fork();
    if(basic_operators == CHILD) {
        printf("BASIC OPERATOR PROCESS STARTED.\n");
        close(P2B[WRITE]);
        
        for(;;) {
            sem_wait(&basic_operators_sem);
            unsigned char size = 0;
            char text[256];

            if(read_from_pipe(P2B[READ], text, &size) == ERROR) break;
            printf("[BASIC OP]: %s\n", text);
        }

        exit(SUCCESS);
    }

    extended_operators = fork();
    if(extended_operators == CHILD) {
        printf("EXTENDED OPERATOR PROCESS STARTED.\n");
        close(P2E[WRITE]);

        for(;;) {
            sem_wait(&extended_operators_sem);
            unsigned char size = 0;
            char text[256];

            if(read_from_pipe(P2E[READ], text, &size) == ERROR) break;
            printf("[EXTENDED OP]: %s\n", text);
        }

        exit(SUCCESS);
    }
    
    int fd = open_file(argv[1]);
    if(fd == ERROR) return ERROR;

    close(P2B[READ]);
    close(P2E[READ]);

    int l = 0;
    char line[256];
    memset(line, 0 , 256);

    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    while(size > 0) {
        read(fd, &line[l], sizeof(char));
        if(line[l] == '\n' && l > 0) {
            dispatch_line(P2B[WRITE], P2E[WRITE], line, strlen(line), &basic_operators_sem, &extended_operators_sem);
            memset(line, 0 , 256);
            l = 0;
        } else l++;

        size--;
    }
    
    printf("Gata!");

    waitpid(basic_operators,    NULL, 0);
    waitpid(extended_operators, NULL, 0);
    return 0;
}

int open_file(char* path) {
    do {
        int fd = open(path, O_RDONLY);
        if(fd < 0) {
            printf("Can't open the file.");
            break;
        }

        return fd;
    } while(1);
    return ERROR;
}

int open_pipe(int fds[2]) {
    do {
        if(pipe(fds) < -1) {
            printf("Can't open the pipe.");
            break;
        }

        return SUCCESS;
    } while(1);
    return ERROR;
}

int write_in_pipe(int fd, char* text, unsigned char size) {
    do {
        int success = write(fd, &size, sizeof(unsigned char));
        if(success != sizeof(unsigned char)) {
            printf("%d", success);
            printf("Can't write the size of the string in the pipe.\n");
            break;
        }

        success = write(fd, text, strlen(text) * sizeof(unsigned char));
        if(success != strlen(text) * sizeof(unsigned char)) {
            printf("Can't write the text in the pipe.\n");
            break;
        }

        return SUCCESS;
    } while(1);
    return ERROR;
}

int read_from_pipe(int fd, char* text, unsigned char* size) {
    do {
        int success = read(fd, size, sizeof(unsigned char));
        if(success != sizeof(unsigned char)) {
            printf("Can't read the size of the string from the pipe.\n");
            break;
        }

        success = read(fd, text, (*size) * sizeof(char));
        if(success != strlen(text) * sizeof(char)) {
            printf("Can't read the text from the pipe.\n");
            break;
        }

        return SUCCESS;
    } while(1);
    return ERROR;
}

int dispatch_line(int fd1, int fd2, char line[256], unsigned char size, sem_t* first, sem_t* second) {
    do {

        printf("LINE TO BE DISPATCHED: %s", line);

        if(strchr(line, '+') || strchr(line, '-')) {
            int success = write_in_pipe(fd1, line, size);
            if(success == ERROR) break;
            sem_post(first);
            printf("Dispatched to basic operators.\n");
        }

        else if(strchr(line, '*') || strchr(line, '/')) {
            int success = write_in_pipe(fd2, line, size);
            if(success == ERROR) break;
            sem_post(second);
            printf("Dispatched to extended operators.\n");
        }

        return SUCCESS;
    } while(1);
    return ERROR;
}