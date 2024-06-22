#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>

#define CHILD 0
#define PROCESS_START(X) printf("[START]: Process %d was launched.\n", X);
#define PRINT_PID(number, EXIT_STATUS, EXIT_STATUS_PARENT) printf("%d: My PID is %d, and my parent's PID is %d.\n", number, EXIT_STATUS, EXIT_STATUS_PARENT)
#define PROCESS_END(X)    { printf("[END]: \t Process %d was stopped.\n", X); exit(0); }

void generateIerarchy();

int main(void) {
    generateIerarchy();
    return 0;
}

void generateIerarchy() {

    pid_t process_2 = fork();
    if(process_2 == CHILD) {
        printf("[MAIN PROCESS]: %d\n", getpid());
        PROCESS_START(2);
        PRINT_PID(2, getpid(), getppid());
        pid_t process_4 = fork();
        if(process_4 == CHILD) {
            PROCESS_START(4);
            PRINT_PID(4, getpid(), getppid());
            PROCESS_END(4);
        } 
    
        waitpid(process_4, NULL, 0);
        pid_t process_3 = fork();
        if(process_3 == CHILD) {
            PROCESS_START(3);
            PRINT_PID(3, getpid(), getppid());
            PROCESS_END(3);
        }

        waitpid(process_3, NULL, 0);
        PROCESS_END(2);
    } else {
        PROCESS_START(1);
        PRINT_PID(1, getpid(), getppid());
        waitpid(process_2, NULL, 0);
        PROCESS_END(1);
    }
   
    return;
}