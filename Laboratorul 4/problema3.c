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

int remove_dir(const char*);

int main(int argc, char** argv) {

    if(argc < 2) {
        PRINT_ERROR(COMMAND_PARAM_ERROR);
        return COMMAND_PARAM_ERROR;
    }

    int status = remove_dir(argv[1]);
    printf("Remove directory function returns %d.\n", status);

    return 0;
}

int remove_dir(const char* path) {

    DIR* current_directory = opendir(path);
    if(path == NULL) {
        PRINT_ERROR(PATH_ERROR);
        return PATH_ERROR;
    }
    struct dirent* current_file = readdir(current_directory);
    struct stat    current_file_stat;

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
            snprintf(current_path, MAX_PATH, "%s/%s", path, current_file->d_name);
            if(lstat(current_path, &current_file_stat) != 0) {
                free(current_path); current_path = NULL;
                continue;
            }
            if(S_ISREG(current_file_stat.st_mode)) remove(current_file->d_name);
            if(S_ISLNK(current_file_stat.st_mode)) unlink(current_file->d_name);
            if(S_ISDIR(current_file_stat.st_mode)) remove_dir(current_path);

            remove(current_path);

            free(current_path); current_path = NULL;
        }
        current_file = readdir(current_directory);
    }
    closedir(current_directory);
    rmdir(path);
    return SUCCESS;
}