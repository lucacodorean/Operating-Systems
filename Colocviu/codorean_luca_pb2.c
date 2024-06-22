#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <ctype.h>

struct stat file_stat_final;
char path_to_file[256];
void* SHARED_MEMORY_FILE = NULL;
int FILE_SIZE = 0;

#define ERROR  -1
#define SUCCESS 1

int read_directory_recursive(char filename[256], char path[256]) {
   
    DIR* current_dir = opendir(path);
    if(current_dir == NULL) {
        printf("Can't open that directory.");
        return -1;
    }
   
    struct dirent* current_file = readdir(current_dir);

    while(current_file != NULL) {
        if(strcmp(current_file->d_name, ".") && strcmp(current_file->d_name, "..")) {

            char file_path[256];
            strcpy(file_path, path);
            strcat(file_path, "/");
            strcat(file_path, current_file->d_name);

            struct stat file_stat; stat(file_path, &file_stat);
            if(strcmp(current_file->d_name, filename) == 0) {
                if(file_stat.st_size > file_stat_final.st_size) {
                    stat(file_path, &file_stat_final);
                    memset(path_to_file, 0, 256);
                    strcpy(path_to_file, file_path);
                }
            }

            if(S_ISDIR(file_stat.st_mode))  read_directory_recursive(filename, file_path);  
        }

        current_file = readdir(current_dir);
    }

    return 0;
}

int open_file(char* path) {

    do {
        int file_descriptor = open(path, O_RDWR);
        if(file_descriptor < 0) {
            perror("Can't open file.");
            break;
        }
        return file_descriptor;
    } while(1);

    return ERROR;
}


int modify(char path_to_file[256]) {
    do {
        int fd = open(path_to_file, O_RDONLY); 
        if(fd < 0) {
            printf("Can't open the found file.\n");
            break;
        }

        int fd_write = open("tempfile.txt", O_WRONLY); 
        if(fd_write < 0) {
            printf("Can't open the found file.\n");
            break;
        }

        int size = lseek(fd, 0, SEEK_END);
        int current = size;
        char c = 0;
        while(size > 0) {
            lseek(fd, -1, SEEK_CUR);
            read(fd, &c, sizeof(char));
            lseek(fd, -1, SEEK_CUR);

            if(!isdigit(c)) write(fd_write, &c, sizeof(char));
            size--;
        }

        close(fd);
        close(fd_write);

        fd = open(path_to_file, O_WRONLY); 
        if(fd < 0) {
            printf("Can't open the found file.\n");
            break;
        }

        fd_write = open("tempfile.txt", O_RDONLY); 
        if(fd_write < 0) {
            printf("Can't open the found file.\n");
            break;
        }

        size = lseek(fd_write, 0, SEEK_END);
        while(size > 0) {
            lseek(fd_write, -1, SEEK_CUR);
            read(fd_write, &c, sizeof(char));
            lseek(fd_write, -1, SEEK_CUR);

            write(fd, &c, sizeof(char));
            size--;
        }

        size = lseek(fd, 0, SEEK_CUR);
        c = '*';
        while(size < current) {
            write(fd, &c, sizeof(char));
            size++;
        }

        return 0;
    } while(1);
    
    return ERROR;
}


int main(int argc, char** argv) {

    if(argc != 3) {
        printf("Can't operate with these parameters.");
        return -1;
    }

    char dir_path[256], filename[256];
    strncpy(dir_path, argv[1], strlen(argv[1]));
    strncpy(filename, argv[2], strlen(argv[2]));


    int new_file = creat("tempfile.txt", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    fstat(new_file, &file_stat_final);

    read_directory_recursive(filename, dir_path);
     close(new_file);

    printf("Fisierul cautat este la path: %s", path_to_file);

    modify(path_to_file);


    close(new_file);
    return 0;
}
