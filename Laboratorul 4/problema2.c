#include <stdio.h>
#include <stdlib.h>

#ifndef __HELPERS__
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <string.h>
    #include <fcntl.h>

    #define __HELPERS__      
    enum RETURN_STATUS {
        SUCCESS             =  0,
        PATH_ERROR          = -1,
        MEMORY_ERROR        = -2,
        SPRINTF_ERROR       = -3,
        NOT_FOUND_ERROR     = -4,
        BUFFER_ERROR        = -5,
        COMMAND_PARAM_ERROR = -6,
        FILE_ERROR          = -7,
        NULL_POINTER_ERROR  = -8,
    };

    static const char ERROR_TEXTS[][50] = {
        "SUCCESS\0",
        "Invalid path to the directory.\0",
        "Can't allocate memory.\0",
        "Can't write in the allocated space.\0",
        "Can't find the specific item.\0",
        "Can't write in the specific buffer.\0",
        "Can't work with the given command parameters.\0",
        "Can't open the file.\0",
        "A pointer is NULL.\0"
    };

    #define TRUE       1
    #define FALSE      0 
    #define MAX_PATH 512
    #define PRINT_ERROR(STATUS_CODE) { printf("Error code %d at line %d: %s", STATUS_CODE, __LINE__, ERROR_TEXTS[(-1) * STATUS_CODE]); }
#endif

size_t create_links(const char*, const char*, const char*, int*);

int main(int argc, char** argv) {
    if(argc < 4) {
        PRINT_ERROR(COMMAND_PARAM_ERROR);
        return COMMAND_PARAM_ERROR;
    }

    int current_link = 0;
    create_links(argv[1], argv[2], argv[3], &current_link);

    return SUCCESS;
}

size_t create_links(const char* directory_path, const char* file_name, const char* string, int* current_link) {

    DIR* current_directory = opendir(directory_path);
    if(current_directory == NULL) {
        PRINT_ERROR(PATH_ERROR);
        return PATH_ERROR;
    }

    struct stat stat_buffer;
    struct dirent* current_file = readdir(current_directory);
    
    while(current_file != NULL) {                                               
        if(strcmp(current_file->d_name, ".") && strcmp(current_file->d_name, "..")) {
            char* current_path = (char*)malloc(MAX_PATH * sizeof(char));

            if(current_path == NULL) {
                PRINT_ERROR(MEMORY_ERROR);
                free(current_path); current_path = 0;
                closedir(current_directory);
                return 0;
            }

            memset(current_path, 0, MAX_PATH * sizeof(char));
            snprintf(current_path, MAX_PATH, "%s/%s", directory_path, current_file->d_name);
            if(lstat(current_path, &stat_buffer) != 0) {
                free(current_path); current_path = NULL;
                continue;
            }

            if(S_ISREG(stat_buffer.st_mode)) {
                if(strcmp(current_file->d_name, file_name) == 0) {
                    int current_file_descriptor = open(current_path, O_RDWR);

                    if(current_file_descriptor < 0) {
                        PRINT_ERROR(FILE_ERROR);
                        free(current_path); current_path = NULL;
                        closedir(current_directory);
                        return FILE_ERROR;
                    }

                    size_t current_file_size = lseek(current_file_descriptor, 0, SEEK_END);
                    lseek(current_file_descriptor, 0, SEEK_SET);
                    char buffer[current_file_size];
                    memset(buffer, 0, current_file_size);

                    size_t read_status = read(current_file_descriptor, buffer, current_file_size);
                    if(read_status != current_file_size) {
                        PRINT_ERROR(BUFFER_ERROR);
                        free(current_path); current_path = NULL;
                        closedir(current_directory);
                        return BUFFER_ERROR;
                    }

                    if(strstr(buffer, string) != NULL) {
                        char link_name[MAX_PATH];
                        sprintf(link_name, "%s.%d", current_file->d_name, *current_link);
                        symlink(current_path, link_name);
                        *current_link = *current_link + 1;
                    }

                    close(current_file_descriptor);
                }
            }

            if(S_ISDIR(stat_buffer.st_mode)) create_links(current_path, file_name, string, current_link);
            free(current_path); current_path = NULL;
        }

        current_file = readdir(current_directory);
    }

    closedir(current_directory);
    return SUCCESS;
}