#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char* argv[])
{
    int fd[2];
    int x = 0;

    if(pipe(fd) != 0) {
        perror("Could not create pipe");
        return 1;
    }

    if(fork() != 0) {
        //parent
        close(fd[0]); //no use for read end
    
        int readS = write(fd[1], argv[1], strlen(argv[1])+1);
        strcat(argv[1], "\0");

        printf("\nParent has written: %s", argv[1]);

        close(fd[1]);
        wait(NULL);
    } else {
   
        close(fd[1]); //no use for write end

        printf("Child receives: \n");

        char c = 'a';
        do {
            read(fd[0], &c, 1);
            printf("%c\n", c);
        } while(c != '\0');
        close(fd[0]);
        }

    return 0;
}