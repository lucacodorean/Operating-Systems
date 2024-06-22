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