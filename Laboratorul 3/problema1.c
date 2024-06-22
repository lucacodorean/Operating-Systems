#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define OPEN_FILE_ERROR 1

int get_line(int, int, char*, int);

int main(void) {

    int file_in = open("/home/lucacodorean/work/lab3/input.txt", O_RDWR);
    if(file_in < 0) {
        perror("File can't be opened.");
        return EXIT_FAILURE;
    }

    char* output = NULL;
    int succes = get_line(file_in, 1, output, 50);
    if(succes == EXIT_SUCCESS) {
        printf("%s", output);
    } 

    free(output); output = NULL;
    return EXIT_SUCCESS;
}

int get_line(int fd, int lineNr, char* line, int maxLength) {

    size_t size_of_file = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char buffer[size_of_file];
    memset(buffer, 0 , size_of_file);

    int READ_STATUS = read(fd, buffer, size_of_file);
    if(READ_STATUS != size_of_file) {
        perror("Reading error.");
        return EXIT_FAILURE;
    }

    int internal_counter = 0;
    char* curr_line = strtok(buffer, "\n");

    while(curr_line != NULL) {
        if(lineNr == internal_counter) {
            line = (char*)malloc(maxLength * sizeof(char));
            if(line == NULL) {
                perror("Could not allocate memory.");
                free(line); line = NULL;
                return EXIT_FAILURE;
            }

            memset(line, 0, maxLength * sizeof(char));

            if(strlen(curr_line) > maxLength) {
                perror("Can't read this line. It's lenght is bigger than maximum accepted");
                free(line); line = NULL;
                return EXIT_FAILURE;
            }

            memcpy(line, curr_line, strlen(curr_line));
            break;
        }
        internal_counter++;
        curr_line = strtok(NULL, "\n");
    }

    close(fd);
    return EXIT_SUCCESS;
}