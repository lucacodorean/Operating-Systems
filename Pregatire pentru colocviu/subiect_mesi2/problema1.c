#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#define START_PROCESS(X) printf("P%d started. PID: %d PPID: %d\n", X, getpid(), getppid());
#define   END_PROCESS(X) printf("P%d   ended. PID: %d PPID: %d\n", X, getpid(), getppid());
#define CHILD 0 
#define WRITE 1
#define READ  0
#define ERROR 1
#define SUCCESS 0
#define SEQUENCE "101101"

int open_nameless_pipe(int*);
int open_file(char*);
int P2C[2], C2P[2];

int main(void) {

    START_PROCESS(1);

    int success = open_nameless_pipe(P2C);
    if(success == ERROR) return ERROR;
    
    success = open_nameless_pipe(C2P);
    if(success == ERROR) return ERROR;

    pid_t  second_process = fork();

    if(second_process == CHILD) {
        
        START_PROCESS(2);
        pid_t forth_process = fork();
        if(forth_process == CHILD) {
            START_PROCESS(4);
            END_PROCESS(4);
            exit(0);
        }

        waitpid(forth_process, NULL, 0);
        pid_t third_process = fork();

        if(third_process == CHILD) {
            START_PROCESS(3);

            close(P2C[WRITE]);
            close(C2P[READ]);

        
            int size_of_received_path = 0;
            if(read(P2C[READ], &size_of_received_path, sizeof(int)) != sizeof(int)) {
                printf("Nu pot sa citesc size-ul din pipe.\n");
                END_PROCESS(3);
                exit(EXIT_FAILURE);
            } 
            
            else {
                char received_path[256];
                memset(received_path, 256, 0);
                if(read(P2C[READ], received_path, size_of_received_path * sizeof(char)) != size_of_received_path * sizeof(char)) {
                    printf("Nu pot sa citesc numele din pipe.\n");
                    END_PROCESS(3);
                    exit(EXIT_FAILURE);
                }

                int fd = open_file(received_path);
                if(fd == ERROR) {
                    END_PROCESS(3);
                    exit(EXIT_FAILURE);
                }

                int size = lseek(fd, 0, SEEK_END);
                lseek(fd, 0, SEEK_SET);
                
                int result = 0;
                char buffer[6];
                for(int i = 0; i<size-6; i++) {
                    memset(buffer, 0, 6);
                    read(fd, buffer, 6);
                    lseek(fd, -5, SEEK_CUR);

                    if(strncmp(buffer, SEQUENCE, strlen(SEQUENCE)) == 0) result++;
                }
                close(fd);

                if(write(C2P[WRITE], &result, sizeof(int)) != sizeof(int)) {
                    printf("Rezultatul nu poate sa fie trimis in pipe-ul C2P.\n");
                    END_PROCESS(3);
                    exit(EXIT_FAILURE);
                } else printf("Rezultatul a fost trimis in pipe-ul C2P.\n");

            }
            END_PROCESS(3);
            exit(SUCCESS);
        }

        close(P2C[READ]);
        close(C2P[WRITE]);

        char path[256];
        memset(path, 256, 0);
        printf("Introduceti numele fisierului: "); scanf("%s", path);

        int size_of_path = strlen(path);

        int status = write(P2C[WRITE], &size_of_path, sizeof(int));
        if(status != sizeof(int)) {
            printf("Size-ul numelui fisierului nu a putut fi trimis pe pipe.\n");
        } else printf("Size-ul numelui fisierului a fost trimis pe pipe-ul P2C.\n");

        status = write(P2C[WRITE], path, strlen(path));
        if(status != strlen(path)) {
            printf("Fisierul nu a putut fi trimis pe pipe.\n");
        } else printf("Numele fisierului a fost trimis pe pipe-ul P2C.\n");

        waitpid(third_process, NULL, 0);

        int rezultat = 0;
        if(read(C2P[READ], &rezultat, sizeof(int)) != sizeof(int)) {
            printf("Rezultatul nu a putut fi citit din C2P.\n");
            END_PROCESS(2);
            exit(EXIT_FAILURE);
        } else printf("[P%d] Secventa %s apare de %d ori in fisierul %s.\n", 2, SEQUENCE, rezultat, path);

        END_PROCESS(2);
        exit(0);
    }

    waitpid(second_process, NULL, 0);
    END_PROCESS(1);

    return 0;
}

int open_nameless_pipe(int fds[2]) {

    do{
        if(pipe(fds) == ERROR) {
            perror("[PIPE CREATION]: Can't create the pipe.\n");
            break;
        }
        return SUCCESS;
    } while(1);

    return ERROR;
}

int open_file(char* path) {
    do {
        int fd = -1;
        if((fd = open(path, O_RDONLY, 0644)) < 0) {
            printf("%d\n", fd);
            printf("[OPEN FILE]: Can't open the file at path %s\n", path);
            break;
        }
        return fd;
    } while(1);

    return ERROR;
}