#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

int main(int argc, char* argv) {

    sem_t sem_temp;
    sem_init(&sem_temp, 1, 1);

    pid_t first_child = fork();


    if(first_child == 0) {
        printf("FIRST CHILD ESTABLISHED.\n");

        pid_t temp = fork();
        if(temp == 0) {
            printf("HE SHOULD RUN AFTER ME.\n");
            printf("I'M DEAD. HE SHOULD BE ABLE TO RUN.\n");
            exit(0);
        }

        waitpid(temp, NULL, 0);
        sem_post(&sem_temp);

        temp = fork();
        if(temp == 0) {
            printf("IM FROM THE FIRST CHILD.\n");
            exit(0);
        }

        exit(0);
    }

    pid_t second_child = fork();
    if(second_child == 0) {
        printf("SECOND CHILD ESTABLISHED.\n");

        sem_wait(&sem_temp);
        pid_t temp = fork();
        if(temp == 0) {
            printf("I RAN CORRECTLY.\n");
            exit(0);
        }

        waitpid(temp, NULL, 0);
        exit(0);
    }

    waitpid(first_child, NULL, 0);
    waitpid(second_child, NULL, 0);

    printf("All done!");
    return 0;
}