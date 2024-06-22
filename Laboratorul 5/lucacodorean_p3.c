#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

int main(int argc, char** argv) {

    srand((int)time(NULL));
    int timeval = rand();
    char* path_dir = (char*)calloc(25, sizeof(char));
    sprintf(path_dir, "directories/%d", timeval);

    char* command = (char*)calloc(50, sizeof(char));
    sprintf(command, "mkdir %s", path_dir);
    system(command);
    free(command); command = NULL;

    DIR* dir = opendir(path_dir);
    if(dir == NULL) {
        return -1;
    }

    int i, n=10;
    for(i=0; i<n; i++) {
        pid_t id = fork();

        char* path_name  = (char*)calloc(50, sizeof(char)); 
        sprintf(path_name, "%s/%d", path_dir, id);
        int fd = creat(path_name, O_WRONLY);
        close(fd);
        free(path_name); path_name = NULL;
    }


    /* A se lua in considerare ultima linie afisata.*/
    command = (char*)calloc(50, sizeof(char));
    sprintf(command, "ls -l %s | wc -l", path_dir);
    system(command);
    free(command); command = NULL;

    //execlp("ls", "ls", "-l", path_dir, NULL);
    return 0;
}