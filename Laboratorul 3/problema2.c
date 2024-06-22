#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv) {

    if(argc < 3) {
        perror("There is no file specified.");
        return -1;
    }

    int file_in = open(argv[1], O_RDONLY);
    if(file_in < 0) {
        perror("The specified argument is not a path to a file.");
        return -1;
    }

    int size_of_file = lseek(file_in, 0, SEEK_END);
    lseek(file_in, 0, SEEK_SET);

    char buffer[size_of_file], buffer_rev[size_of_file];
    memset(buffer, 0, size_of_file);
    memset(buffer_rev, 0, size_of_file);

    char command[45 + size_of_file];
    memset(buffer, 0, size_of_file);
    int READ_STATUS = read(file_in, buffer, (size_of_file-1) * sizeof(char));
    if(READ_STATUS != size_of_file-1) {
        perror("Error while reading from file.");
        return -1;
    }

    int file_out = open(argv[2], S_IRUSR | S_IWUSR);
    if(file_out < 0) {
        perror("Error at creating the output file:");
        close(file_in);
        return EXIT_FAILURE;
    }

    char* curr_line = strtok(buffer, "\n");
    while(curr_line != NULL) {
        char command[50 + strlen(curr_line)];
        memset(command, 0, 50 + strlen(curr_line));
        strcat(command, "`LC_ALL=UTF-8 &`");
        strcat(command, "echo ");
        strcat(command, curr_line);
        strcat(command, " | rev | ");
        strcat(command, argv[2]);
        system(command);
        curr_line = strtok(NULL, "\n");
    }

    return 0;
}