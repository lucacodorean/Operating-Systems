#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef __CONSTANTS__
    #define MAX_PATH          1024
    #define RECURSIVE_FLAG       1
    #define SIZE_SMALLER_FLAG    4
    #define HAS_WRITE_PERMS_FLAG 2
    #define FLAG_CHECKED         1
    #define FLAG_UNCHECKED       0
    #define TRUE                 1
    #define FALSE                0

    enum STATUS_CODES{
        SUCCESS                 =  0,
        GENERAL_ERROR           = -1,
        READ_ERROR              = -2,
        MEMORY_ALLOC_ERROR      = -3,
        FILE_ERROR              = -4,
        PARAMETER_NOT_ACCEPTED  = -5,
        DIRECTORY_PATH_ERROR    = -6
    };

    #define PRINT_ERROR(X) { printf("Error code: %d at line %d.\n", X, __LINE__); }
#endif

#ifndef __SPECIAL_FILE__
    #define SECTION_NAME_SIZE          14
    #define SECTION_SEPARATOR      0x0A0D
    #define SECTION_NUMBER_LOWER_BOUND  6
    #define SECTION_NUMBER_UPPER_BOUND 20
    #define SF_MAGIC_SIZE               2
    #define VERSION_LOWER_BOUND      0x28
    #define VERSION_UPPER_BOUND      0x63
    #define MAGIC_SEQUENCE           "MG"

    typedef struct __attribute__((packed)) {
        short sec_type; 
        int section_offset, section_size;
        char* section_name;
    } HEADER_SECTION;

    HEADER_SECTION* init_section_header();
    void free_section_header_data(HEADER_SECTION*);

    typedef struct __attribute__((packed)) {
        short header_size;
        short number_of_sections;
        short version;
        char* magic;
        HEADER_SECTION** sections;
    } HEADER;         

    typedef struct __attribute__((packed)) node {
        char* text;
        struct node* next;
    } NODE;

    NODE* new_node(int, char*);
    void  add_to_list(NODE**, int, char*);
    void  free_list(NODE**);

    HEADER* init_header(short, short, short);
    HEADER* parse(char*, int);
    void free_header_data(HEADER*);
    void print_header_details(HEADER*);
#endif

#ifndef __HELPER_FUNCTIONS__
    int   get_directory_specs(char*, short, int, int);       /* IMPLEMENTS LIST COMMAND FUNCTIONALITY.    */                                     
    char* get_file_path(char*, struct dirent*);              /* IMPLEMENTS THE RELATIVE PATH FILE LOGIC.  */
    int   string_to_integer(char*);                          /* IMPLEMENTS A STRING TO INTEGER CONVERSION.*/
    char* extract_path(char*);                               /* IMPLEMENTS THE PATH EXTRACTION FROM COMMAND.*/
    int valid_section_type(int);
#endif

int   extract(char*, HEADER*, int, int);
int   find_all(char*, int);

int main(int argc, char **argv)
{
    if(argc >= 2) {
        if(strcmp(argv[1], "variant") == 0) {
            printf("14118\n");
        }

        if(strcmp(argv[1], "list") == 0) {

            short FLAGS = 256;
            int   value = 0;
            char* path  = (char*)calloc(MAX_PATH/2, sizeof(char));
            int   status = 0;

            for(int i = 2; i<argc; i++) {
                if(strcmp(argv[i], "recursive") == 0) 
                    FLAGS = FLAGS | RECURSIVE_FLAG;

                else if(strcmp(argv[i], "has_perm_write") == 0) {
                    FLAGS = FLAGS | HAS_WRITE_PERMS_FLAG;
                }

                else if(strstr(argv[i], "size_smaller=") != 0) {
                    FLAGS = FLAGS | SIZE_SMALLER_FLAG;
                    value = string_to_integer(argv[i]);
                }

                else if(strstr(argv[i], "path=") != 0) strcpy(path, extract_path(argv[i]));
            }

            if(path != NULL) {
                status = get_directory_specs(path, FLAGS, value, 0);
                free(path); path = NULL;
            } 

            if(status == GENERAL_ERROR) {
                printf("ERROR\ninvalid directory path");
            }

            free(path); path = NULL;
        }

        if(strcmp(argv[1], "parse") == 0) {
            char* path  = (char*)calloc(MAX_PATH/2, sizeof(char));
            if(strstr(argv[2], "path=") != 0) strcpy(path, extract_path(argv[2]));

            if(path != NULL) {
                HEADER* file_header = parse(path, 0);
                print_header_details(file_header);
                free_header_data(file_header); file_header = NULL;
            }

            free(path); path = NULL;
        }

        if(strcmp(argv[1], "extract") == 0) {
            char* path = (char*)calloc(MAX_PATH/2, sizeof(char));
            
            if(strstr(argv[2], "path=") != 0)   strcpy(path, extract_path(argv[2]));

            if(path != NULL) {
                int section = -1, line = -1;
                HEADER* file_header = parse(path, 0);

                if(strstr(argv[3], "section=") !=0) {
                    section = string_to_integer(argv[3]);
                    if(section > file_header->number_of_sections) {
                        printf("ERROR\ninvalid section");
                    } 
                    else {
                        if(strstr(argv[4], "line=")) line = string_to_integer(argv[4]);
                        if(line != -1 && section != -1) extract(path, file_header, section, line);
                    }
                }

                free_header_data(file_header); file_header = NULL;
            } else printf("ERROR\ninvalid file");
            free(path); path=NULL;
        }

        if(strcmp(argv[1], "findall") == 0 ) {
            char* path = (char*)calloc(256, sizeof(char));
            if(strstr(argv[2], "path=") != 0) strcpy(path, extract_path(argv[2]));

            if(path != NULL) find_all(path, 0);
            free(path); path = NULL;
        }
    }
    return SUCCESS;
}

int get_directory_specs(char* path, short FLAGS, int REQ_SIZE, int succes) {    /// ASSUME THE . AND .. DIRECTORIES ARE AVOIDED
    DIR* current_dir = opendir(path);                                           /// OPEN THE DIRECTORY FOUND AT THE PATH SPECIFIED PATH
    
    if(current_dir == NULL) return -1;                                          /// IF THE CURRENT_DIR IS NULL

    if(succes == 0) {                                                           /// DISPLAY THE SUCCESS TEXT AS REQUIRED
        printf("SUCCESS\n");
        succes = 1;
    }

    struct dirent* current_file = readdir(current_dir);                         /// GET THE CURRENT FILE FROM THE OPENED DIRECTORY
    
    while(current_file != NULL) {                                               /// START AN ITERATION OVER THE CURRENT DIRECTORY
        if(strcmp(current_file->d_name, ".") &&                                 /// CHECK IF THE CURRENT DIRECTORY IS . OR .. AND VOID PRINTING THEM.
           strcmp(current_file->d_name, "..")) {

            char* relative_path = get_file_path(path, current_file);            /// SAVE THE PATH RETURNED BY THE FUNCTION get_file_path
            struct stat current_directory_stats;                                /// DECLARING A VARIABLE THAT KEEPS TRACK OF CURRENT_FILE'S METADATA.
            stat(relative_path, &current_directory_stats);                      /// GET THE METADATA FOR THE CURRENT_FILE
            
            if((FLAGS & HAS_WRITE_PERMS_FLAG)) {                                /// CHECK IF THE HAS_WRITE_PERMS FLAG IS ACTIVE
                if((current_directory_stats.st_mode & S_IWUSR)) {               /// CHECK THE WRITING PERMISSION BY OWNER
                    printf("%s\n", relative_path);                              /// PRINT THE PATH ACCORDINGLY
                }
            }

            else if((FLAGS & SIZE_SMALLER_FLAG)) {                              /// CHECK IF THE SIZE_SMALLER FLAG IS ACTIVE
                if(current_directory_stats.st_size < REQ_SIZE &&                /// IF IT IS, VERIFY DATA'S SIZE
                    S_ISREG(current_directory_stats.st_mode)) {                 /// ALSO VERIFY DATA'S FILE TYPE
                    printf("%s\n", relative_path);                              /// PRINT THE PATH ACCORDINGLY
                }
            } 
            
            else printf("%s\n", relative_path);                                 /// PRINT THE PATH IF THE WRITE_PERMS_FLAG IS NOT ACTIVE
    
            if((FLAGS & RECURSIVE_FLAG) == 1 &&                                 /// EVALUATING IF RECURSIVE FLAG IS ACTIVE
                S_ISDIR(current_directory_stats.st_mode)) {                     /// EVALUATING IF THE THE CURRENT_FILE IS A DIRECTORY
                get_directory_specs(relative_path, FLAGS, REQ_SIZE, succes);    /// RECURSIVE CALL TO ITERATE THROUGH THE CURRENT_FILE 
            }                                                                   /// WHICH IS A DIRECTORY

            free(relative_path);                                                /// FREE THE MMEMORY ALLOCATED FOR TEMP-STORTING THE PATH.
            relative_path = NULL;
        }

        current_file = readdir(current_dir);                                    /// ADVANCE TO THE NEXT FILE.
    }

    closedir(current_dir);                                                      /// CLOSING THE DIRECTORY
    return SUCCESS;                                                       /// RETURNING THE 0 STATUS CODE FOR SUCCES
}

char* get_file_path(char* path, struct dirent* current_file) {                  /// FUNCTION THAT HANDLES PATH FORMATION
    char* result = (char*)calloc(MAX_PATH, sizeof(char));                       /// MEMORY ALLOCATION FOR THE PATH
    snprintf(result, MAX_PATH, "%s/%s", path, current_file->d_name);            /// GENERATING THE PATH
    return result;                                                              /// REURNING THE PATH
}

int string_to_integer(char* text) {
   
    char* temp = strtok(text, "=");
    temp = strtok(NULL, "=");
    strcat(temp, "\0");

    int result = 0, i = 0;
    
    while(text[i] != '\0') {
        if(temp[i] < 48 || temp[i] >  57) break;    
        result = result*10 + (temp[i++] - 48);
    }

    return result;
}

char* extract_path(char* path_text) {
    char* temp = strtok(path_text, "=");
    temp = strtok(NULL, "=");
    strcat(temp, "\0");
    return temp;
}

int valid_section_type(int section_type) {
    return(
        (section_type == 79 || section_type == 42 || section_type == 76 ||
         section_type == 25 || section_type == 27) ? TRUE : FALSE
    );
}

void free_section_header_data(HEADER_SECTION* header_section) {
    if(header_section == NULL) return; 
    
    if(header_section->section_name != NULL) {
        free(header_section->section_name); 
        header_section->section_name = NULL;
    }

    free(header_section); header_section = NULL;
}

void free_header_data(HEADER* header) {
    if(header == NULL) return;

    if(header->sections != NULL) {
        for(int i = 0; i<header->number_of_sections; i++) {
            if(header->sections[i] == NULL) continue;
            free_section_header_data(header->sections[i]); 
        }

        free(header->sections); header->sections = NULL;
    }

    if(header->magic != NULL) {
        free(header->magic);    
        header->magic = NULL;
    }

    free(header); header = NULL;
}

void print_header_details(HEADER* current_header) {
    if(current_header == NULL) return;

    printf("SUCCESS\n");
    printf("version=%d\n",     current_header->version);
    printf("nr_sections=%d\n", current_header->number_of_sections);
    for(int i = 0; i<current_header->number_of_sections; i++) {
        if(current_header->sections[i] == NULL || current_header->sections[i]->section_name == NULL) continue;
        printf("section%d: %s %d %d\n", 
            i+1, 
            current_header->sections[i]->section_name,
            current_header->sections[i]->sec_type,
            current_header->sections[i]->section_size
        );
    }
}

HEADER* parse(char* path, int silent) {
    int file = open(path, O_RDONLY);
    char magic[3];
    int CURRENT_READ_VALUE = 0;
    int status =  -1;
    HEADER* header = NULL;

    if(file < 1) {
        return NULL;
    }

    lseek(file, -2, SEEK_END);
    CURRENT_READ_VALUE = read(file, magic, SF_MAGIC_SIZE);
    strcat(magic, "\0");

    if(CURRENT_READ_VALUE != 2) {
        if(silent == FALSE) PRINT_ERROR(READ_ERROR);
        return NULL;
    }

    if(strcmp(magic, MAGIC_SEQUENCE) != 0) {
        if(silent == FALSE) {
            printf("ERROR\nwrong magic");
        }
        close(file);
        return NULL;
    }
    short version = 0, section_number = 0, header_size = 0;

    // dupa citirea lui MAGIC, cursorul se afla din nou la SEEK_END, iar pentru a accesa
    // HEADER_SIZE, trebuie sa ne deplasam cu -4 de la SEEK_END intrucat MAGIC_SIZE ocupa 2 octeti
    // folosim sizeof(short) intrucat marimea este ecvhivalenta cu 2 octeti

    lseek(file, -4, SEEK_END); 
    CURRENT_READ_VALUE = read(file, &header_size, sizeof(short));
    if(CURRENT_READ_VALUE != 2) {
        close(file);
        return NULL;
    }
    
    lseek(file,  0, SEEK_END); 

    /// ne-am deplasat la sfarsitul fisierului, ulterior mergem la inceputul headerului
    /// de unde citim versiunea, care poate avea maxim 2 octeti, deci sizeof(short)

    lseek(file, -header_size, SEEK_END); 
    CURRENT_READ_VALUE = read(file, &version, sizeof(short));
    if(CURRENT_READ_VALUE != 2) {
        if(silent == FALSE) PRINT_ERROR(MEMORY_ALLOC_ERROR);
        close(file);
        return NULL;
    }

    if(version < VERSION_LOWER_BOUND || version > VERSION_UPPER_BOUND) {
        if(silent == FALSE) {
            printf("ERROR\nwrong version");
        }
        close(file);
        return NULL;
    }

    CURRENT_READ_VALUE = read(file, &section_number, sizeof(short)/2);

    if(CURRENT_READ_VALUE != 1) {
        if(silent == FALSE) PRINT_ERROR(MEMORY_ALLOC_ERROR);
        close(file);
        return NULL;
    }

    if(section_number != 2 && 
      (section_number < SECTION_NUMBER_LOWER_BOUND || section_number > SECTION_NUMBER_UPPER_BOUND)) {
        if(silent == FALSE) {
            printf("ERROR\nwrong sect_nr");
        }
        close(file);
        return NULL;
    }

    do {
        header = (HEADER*)malloc(sizeof(HEADER));
        if(header == NULL) {
            if(silent == FALSE) PRINT_ERROR(MEMORY_ALLOC_ERROR);
            status = -1;
            break;
        }

        status = 0;
        memset(header, 0, sizeof(HEADER));

        header->header_size = header_size;
        header->number_of_sections = section_number;
        header->version = version;

        header->sections = (HEADER_SECTION**)malloc((header->number_of_sections + 6)* sizeof(HEADER_SECTION*));
        
        if(header->sections == NULL) {
            if(silent == FALSE) PRINT_ERROR(MEMORY_ALLOC_ERROR);
            status = -1;
            break;
        }

        memset(header->sections, 0, sizeof(HEADER_SECTION*) * header->number_of_sections);

        for(int i = 0; i<header->number_of_sections; i++) {
                header->sections[i] = (HEADER_SECTION*)malloc(sizeof(HEADER_SECTION));

                if(header->sections[i] == NULL) {
                    if(silent == FALSE) PRINT_ERROR(MEMORY_ALLOC_ERROR);
                    status = -1;
                    break;
                }

                memset(header->sections[i], 0, sizeof(HEADER_SECTION));

                header->sections[i]->section_name = realloc(header->sections[i]->section_name, SECTION_NAME_SIZE * sizeof(char));
                CURRENT_READ_VALUE = read(file, header->sections[i]->section_name, SECTION_NAME_SIZE * sizeof(char));
                if(CURRENT_READ_VALUE != SECTION_NAME_SIZE * sizeof(char)) {
                    if(silent == FALSE) PRINT_ERROR(READ_ERROR);
                    free_header_data(header);
                    status = -1;
                    break;
                }
              

                header->sections[i]->sec_type = 0;
                CURRENT_READ_VALUE = read(file, &header->sections[i]->sec_type, sizeof(short)/2);
                if(CURRENT_READ_VALUE != sizeof(short)/2) {
                    if(silent == FALSE) PRINT_ERROR(READ_ERROR);
                    free_header_data(header);
                    status = -1;
                    break;
                }
                
                short temp = header->sections[i]->sec_type;
                if(!(temp == 79 || temp == 42 || temp == 76 || temp == 25 || temp == 27)) {
                    if(silent == FALSE) printf("ERROR\nwrong sect_types");
                    free_header_data(header);
                    return NULL;
                }

                CURRENT_READ_VALUE = read(file, &header->sections[i]->section_offset, sizeof(header->sections[i]->section_offset));
                if(CURRENT_READ_VALUE != sizeof(header->sections[i]->section_offset)) {
                    if(silent == FALSE) PRINT_ERROR(READ_ERROR);
                    free_header_data(header);
                    status = -1;
                    break;
                }

                CURRENT_READ_VALUE = read(file, &header->sections[i]->section_size, sizeof(header->sections[i]->section_size));
                if(CURRENT_READ_VALUE != sizeof(header->sections[i]->section_offset)) {
                    if(silent == FALSE) PRINT_ERROR(READ_ERROR);
                    free_header_data(header);
                    status = -1;
                    break;
                }
        }

    } while(FALSE);

    if(status == -1) {
        if(silent == FALSE) PRINT_ERROR(MEMORY_ALLOC_ERROR);
        return NULL;
    }

    return header;
}

void free_list(NODE** list) {
    NODE* temp = *list;
    NODE* aux = NULL;

    while(temp != NULL) {
        aux = temp->next;
        free(temp->text); temp->text = NULL;
        free(temp);
        temp = aux;
    }

    *list = NULL;
} 

NODE* new_node(int value, char paragraph[]) {
    int status = 0;
    NODE* temp = NULL;
    do {
        temp = (NODE*)malloc(sizeof(NODE));
        if(temp == NULL) {
            status = -1;
            break;
        }

        status = 0;
        memset(temp, 0, sizeof(NODE));
        
        temp->text = (char*)malloc((value)*sizeof(char));
        if(temp->text == NULL) {
            status = -1;
            break;
        }
        
        memset(temp->text, 0, (value)*sizeof(char));
        strncpy(temp->text, paragraph, value);
        temp->next = NULL;
    } while(FALSE);
    
    if(status ==  -1) {
        free(temp->text); temp->text = NULL;
        free(temp); temp = NULL;
    }

    return temp;
}

void add_to_list(NODE** list, int value, char paragraph[]) {
    if(*list == NULL) {
        *list = new_node(value, paragraph);
        return;
    } 

    NODE* newNode = new_node(value, paragraph);
    newNode->next = (*list);
    (*list) = newNode;
    return;
}

int extract(char* path, HEADER* file_header, int section_index, int line_index) { 

    int file = open(path, O_RDONLY);
    if(file < 0) {
        printf("ERROR\ninvalid file");
        return FILE_ERROR;
    }

    HEADER_SECTION* current_section = file_header->sections[section_index - 1];

    if(current_section == NULL) {
        free_section_header_data(current_section); current_section = NULL;
        return MEMORY_ALLOC_ERROR;
    }

    NODE* line_list = NULL;

    lseek(file, current_section->section_offset, SEEK_SET);

    char buffer[current_section->section_size +1];
    int EVALUATE_READ = read(file, buffer, (current_section->section_size + 1) * sizeof(char));

    if(EVALUATE_READ != (current_section->section_size + 1) * sizeof(char)) {
        free_section_header_data(current_section);
        return READ_ERROR;
    }

    char* backup = buffer;
    char separator[3] = {0x0d, 0x0a, 0x00};
    char* line = strtok_r(buffer, separator, &backup);

    int line_counter = 0;

    while(line != NULL) {
        line_counter++;
        add_to_list(&line_list, strlen(line), line);
        line = strtok_r(NULL, separator, &backup);
    }

    if(line_index > line_counter) {
        printf("ERORR\ninvalid line");
        free_section_header_data(current_section);
        free_list(&line_list);
        return PARAMETER_NOT_ACCEPTED;
    }

    int line_index_to_be_displayed = line_counter - line_index;

    int counter = 0;
    NODE* temp = line_list;
    while(temp != NULL) {
        if(counter == line_index_to_be_displayed) {
            
            system("echo SUCCESS\n");
            char command[33 + strlen(temp->text)];
            memset(command, 0, 33 + strlen(temp->text));
            strcat(command, "`LC_ALL=UTF-8 &`");
            strcat(command, "echo ");
            strcat(command, temp->text);
            strcat(command, " | rev");
            system(command);
            break;
        } 
        counter++;
        temp = temp->next;
    }

    free_section_header_data(current_section);
    free_list(&line_list); 
    return SUCCESS;
}

int find_all(char* path, int success) {
    DIR* current_directory = opendir(path);

    if(current_directory == NULL) {
        return GENERAL_ERROR;
    }

    if(success == 0) {
        printf("SUCCESS\n");
        success = 1;
    }

    struct dirent* current_file = readdir(current_directory); 

    while(current_file != NULL) {
        if(strcmp(current_file->d_name, ".") &&                                 
           strcmp(current_file->d_name, "..")) {

            char* relative_path = get_file_path(path, current_file);            
            struct stat current_directory_stats;                            
            stat(relative_path, &current_directory_stats);                     

            if(S_ISDIR(current_directory_stats.st_mode)) find_all(relative_path, success);
            else if(S_ISREG(current_directory_stats.st_mode)) {
                HEADER* current_header = parse(relative_path, 1);
                if(current_header != NULL) {
                    int total_27 = 0;

                    for(int cs_index = 0; cs_index < current_header->number_of_sections; cs_index++) {
                        if(current_header->sections[cs_index]->sec_type == 27) total_27++;
                        if(total_27 == 2) break;
                    }
                    
                    if(total_27 == 2) printf("%s\n", relative_path);
                }

                free_header_data(current_header);  current_header = NULL;
            }

            free(relative_path);  relative_path = NULL;
        }
        current_file = readdir(current_directory);  
    }

    free(current_file); current_file = NULL;
    closedir(current_directory);
    return SUCCESS;
}