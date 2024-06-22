#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>

#define PROCEESS_START() printf("Start pid=%d\n", getpid());
#define PROCEESS_END()     printf("End pid=%d\n", getpid());

#define CHILD  0
#define LIMIT 50

int main(void) {

    PROCEESS_START();
    pid_t second_process = fork();

    if(second_process == CHILD) {
        PROCEESS_START();
        PROCEESS_END();

        pid_t third_process = fork();
        if(third_process == CHILD) {
            PROCEESS_START();

            PROCEESS_END();
            srand(time(NULL));
            exit(rand() % LIMIT);
        }

        pid_t forth_process = fork();
        if(forth_process == CHILD) {
            PROCEESS_START();

            PROCEESS_END();
            srand(time(NULL));
            exit(rand() % LIMIT);
        }

        pid_t sixth_process = fork();
        if(sixth_process == CHILD) {
            PROCEESS_START();

            PROCEESS_END();
            srand(time(NULL));
            exit(rand() % LIMIT);
        }

        int status3, status4, status6;

        waitpid(third_process, &status3, 0);
        waitpid(forth_process, &status4, 0);
        waitpid(sixth_process, &status6, 0);
        exit(WEXITSTATUS(status3) + WEXITSTATUS(status4) + WEXITSTATUS(status6));
    }

    pid_t fifth_process = fork();
    if(fifth_process == CHILD) {
        PROCEESS_START();

        PROCEESS_END();
        srand(time(NULL));
        exit(rand() % LIMIT);
    }

    int status2, status5;
    waitpid(second_process, &status2, 0);
    waitpid(fifth_process,  &status5, 0);
    printf("Suma tuturor frunzelor este: %d\n", WEXITSTATUS(status2) + WEXITSTATUS(status5));
    PROCEESS_END();

    return 0;
}