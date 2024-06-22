#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(void)
{
    int fdP2C[2];
    char c = 0;
    int i;

    sem_t* semaphore_parent_child = sem_open("PARENT_CHILD", O_CREAT, 0644, 0);
    sem_t* semaphore_child_parent = sem_open("CHILD_PARENT", O_CREAT, 0644, 0);


    if(pipe(fdP2C) != 0) {
        perror("Could not create pipes");
        return 1;
    }

    if(fork() != 0) {
        //parent
        close(fdP2C[0]);
        c = 'a';
        for(i=0; i<10; i++) {
            sem_post(semaphore_child_parent);
            write(fdP2C[1], &c, sizeof(c));
            sem_wait(semaphore_child_parent);
            sem_wait(semaphore_parent_child);
            read(fdP2C[0], &c, sizeof(c));
            printf("Parent: %c\n", c);
            c++;
        }
        close(fdP2C[1]);
        wait(NULL);
    } else {
        //child
        close(fdP2C[1]);
        for(i=0; i<10; i++) {
            sem_post(semaphore_parent_child);
            read(fdP2C[0], &c, sizeof(c));
            printf("Child: %c\n", c);
            c++;
            sem_wait(semaphore_child_parent);
            write(fdP2C[1], &c, sizeof(c));
            sem_post(semaphore_child_parent);
        }
        close(fdP2C[0]);
    }

    // sem_destroy(semaphore_child_parent);
    // sem_destroy(semaphore_parent_child);

    return 0;
}