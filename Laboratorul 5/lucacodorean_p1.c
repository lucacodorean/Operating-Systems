#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PRINT_PID(number, EXIT_STATUS, EXIT_STATUS_PARENT) printf("%d: My PID is %d, and my parent's PID is %d.\n", number, EXIT_STATUS, EXIT_STATUS_PARENT)

int main(int argc, char** argv) {


    pid_t second_process, third_process;
    int second_process_status = 0, third_process_status = 0;
    int fifth_process_status = 0, forth_process_status = 0;
    printf("THE ID FOR PSTREE IS: %d.\n", getpid());
    second_process = fork();

    if(second_process == 0) {
        pid_t forth_process = fork();

        if(forth_process == 0) {
            PRINT_PID(4, getpid(), getppid());
            sleep(60);
            exit(0);
        }

        waitpid(forth_process, &forth_process_status, 0);
        PRINT_PID(2, getpid(), getppid());
        exit(0);
    }

    third_process = fork();
    if(third_process == 0) {
        pid_t fifth_process = fork();

        if(fifth_process == 0) {
            PRINT_PID(5, getpid(), getppid());
            sleep(60);
            exit(0);
        }
        waitpid(fifth_process, &fifth_process_status, 0);
        PRINT_PID(3, getpid(), getppid());
        exit(0);
    }

    waitpid(second_process, &second_process_status, 0);
    waitpid(third_process,  &third_process_status, 0);
    printf("I am P1 and my PID is %d.\n", getpid());

    return 0;
}