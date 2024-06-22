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

    typedef struct node {
        __ino_t inode;
        struct node* next;
    } NODE;

    NODE* new_node(__ino_t);
    void  add_to_list(NODE**, __ino_t);
    int   find_path(NODE*, __ino_t);
    void  free_list(NODE**);
#endif

NODE* checked_files = NULL;
__off_t dirSize(const char*);

int main(int argc, char** argv) {

    if(argc < 2) {
        PRINT_ERROR(PATH_ERROR);
        return PATH_ERROR;
    }

    __off_t result = dirSize(argv[1]);
    if(result >= 0) printf("Size of %s directory is %ld." , argv[1], result);
    else return -1;

    free_list(&checked_files);

    return SUCCESS;
}

NODE* new_node(__ino_t param) {
    int status = SUCCESS;
    NODE* temp = NULL;
    do {
        temp = (NODE*)malloc(1 * sizeof(NODE));

        if(temp == NULL) {
            status = -1;
            PRINT_ERROR(MEMORY_ERROR);
            break;
        }

        memset(temp, 0, sizeof(temp));
        temp->inode = param;
        temp->next = NULL;

    } while(FALSE);

    if(status != SUCCESS) {
        free(temp); 
        temp = NULL;
    }

    return temp;
}

void add_to_list(NODE** list, __ino_t param) {
    if(*list == NULL) {
        *list = new_node(param);
        return;
    }

    NODE* temp = new_node(param);
    temp->next = (*list);
    (*list) = temp;
    return;
}

int find_path(NODE* list, __ino_t param) {
    NODE* temp = list;
    while(temp != NULL) {
        if(temp->inode == param) {
            return SUCCESS;
        }

        temp = temp->next;
    }

    return NOT_FOUND_ERROR;
}

void free_list(NODE** list) {
    NODE* temp = *list;
    while(temp != NULL) {
        NODE* aux = temp->next;
        free(temp); 
        temp = aux;
    }
}

__off_t dirSize(const char* dirPath) {
    DIR* current_directory = opendir(dirPath);

    if(current_directory == NULL) {
        PRINT_ERROR(PATH_ERROR);
        return PATH_ERROR;
    }

    __off_t result = 0;
    struct stat statbuffer;
    struct dirent* current_file = readdir(current_directory);

    while(current_file != NULL) {                                               
        if(strcmp(current_file->d_name, ".") &&                                
           strcmp(current_file->d_name, "..")) {
                char* current_path = (char*)malloc(MAX_PATH * sizeof(char));

                if(current_path == NULL) {
                    PRINT_ERROR(MEMORY_ERROR);
                    free(current_path); current_path = 0;
                    closedir(current_directory);
                    return 0;
                }

                memset(current_path, 0, MAX_PATH * sizeof(char));
                snprintf(current_path, MAX_PATH, "%s/%s", dirPath, current_file->d_name);
                lstat(current_path, &statbuffer);

                if(find_path(checked_files, statbuffer.st_ino) == NOT_FOUND_ERROR) {
                    if(S_ISREG(statbuffer.st_mode)) result += statbuffer.st_size;
                    
                    else if(S_ISLNK(statbuffer.st_mode)) {
                        char* buffer = (char*)malloc(MAX_PATH * sizeof(char));

                        if(buffer == NULL) {
                            PRINT_ERROR(MEMORY_ERROR);
                            free(current_path); current_path = 0;
                            closedir(current_directory);
                            return 0;
                        }

                        memset(buffer, 0, MAX_PATH * sizeof(char));
                        int read_link = readlink(current_path, buffer, MAX_PATH * sizeof(char));

                        if(read_link != strlen(buffer)) {
                            PRINT_ERROR(BUFFER_ERROR);
                            free(buffer); buffer = NULL;
                            free(current_path); current_path = 0;
                            closedir(current_directory);
                            return 0;
                        }

                        struct stat readlink_buffer;
                        if(lstat(buffer, &readlink_buffer) != 0) {
                            free(buffer); buffer = NULL;
                            continue;
                        }
                        
                        result += readlink_buffer.st_size;
                    }

                    add_to_list(&checked_files, statbuffer.st_ino);
                }


                if(S_ISDIR(statbuffer.st_mode)) result += dirSize(current_path);

                free(current_path);
                current_path = NULL;

        }

        current_file = readdir(current_directory);                              
    }


    closedir(current_directory);
    current_directory = NULL;
    return result;
}