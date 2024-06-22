#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <limits.h>

#ifndef __HELPERS__H
    #define __HELPERS_H

    #define READ_PIPE_NAME                  "REQ_PIPE_14118"
    #define WRITE_PIPE_NAME                 "RESP_PIPE_14118"
    #define INIT_CONNECTION_STRING_FIELD    "START"
    #define VARIANT_NUMBER                  14118
    #define SHM_SIZE                        2246117
    #define SHM_NAME                        "/qnTELAaw"
    #define SHM_PERMS                       0664
    #define LOGICAL_OFFSET_PAGE_SIZE        2048
    #define SUCCESS                         0
    #define ERROR                          -1
    #define FALSE                           0 
    #define TRUE                            1

    #define LENGTH_AND_STRING(X) strlen(X), X
    #define ASSIGN_STRING_FIELD(field, X) { field.size = strlen(X); strcpy(field.contents, X); }
    #define EVALUATE_BREAK(condition, evaluator)  if(condition != evaluator) { printf("[EVALUATION] Break happening at line: %d.\n", __LINE__); break; } 
    #define EVALUATE_ERROR(condition, evaluator)  if(condition == evaluator) { printf("[EVALUATION] Probably error happening at line: %d.\n", __LINE__);  break; } 

    int open_pipe(char*, int);
    int create_pipe(char*);
    void close_pipe(int, char*);

    typedef struct __attribute__((packed)) {
        unsigned char size;
        char contents[256];
    } STRING_FIELD;

    void          free_string_field(STRING_FIELD*);
    int           send_over_pipe(int, STRING_FIELD);
    void          display_string_field(STRING_FIELD*);
    STRING_FIELD* init_string_field(unsigned char, char*);
    int           read_from_pipe(int, char*, unsigned short, unsigned char*);
#endif

#ifndef __SPECIAL_FILE_IMPLEMENTATION__H
    #define __SPECIAL_FILE_IMPLEMENTATION__H

    #define SECTION_NAME_SIZE          14
    #define HEADER_SECTION_SIZE        23
    #define SECTION_SEPARATOR      0x0A0D
    #define SECTION_NUMBER_LOWER_BOUND  6
    #define SECTION_NUMBER_UPPER_BOUND 20
    #define SF_MAGIC_SIZE               2
    #define VERSION_LOWER_BOUND      0x31
    #define VERSION_UPPER_BOUND      0x63
    #define HEADER_SIZE                35

    typedef struct __attribute__((packed)) {
        short sec_type; 
        int section_offset, section_size;
        char section_name[SECTION_NAME_SIZE];
    } HEADER_SECTION;

    HEADER_SECTION* init_section_header();
    void free_section_header_data(HEADER_SECTION*);

    typedef struct __attribute__((packed)) {
        short header_size;
        short number_of_sections;
        short version;
        char* magic;
        HEADER_SECTION sections[256];
    } HEADER;    

    void          print_header_details(HEADER*); 
    int           parse(int, int, HEADER*);
    void          free_header_data(HEADER*);
#endif

#ifndef __TASKS_SOLUTIONS__H
    #define __TASKS_SOLUTIONS__H

    int           create_shm();
    int           put_file_in_shm(char*, int*);
    int           initialize_connection(char*, char*, int*, int*);
    int           write_in_memory(unsigned int, unsigned int, int);
    int           read_bytes_from_offset(unsigned int, unsigned int, int);
    int           read_from_file_header(HEADER, unsigned int, unsigned int, unsigned int, int);
    int           read_from_logical_space_offset(HEADER, unsigned int, unsigned int, int);

    void* SHARED_MEMORY_ADDRESS = NULL;
#endif

int main(void) {

    int READ_PIPE  = -1, WRITE_PIPE = -1, FINISHED = 0, SHARED_MEMORY, FILE_DESCRIPTOR;
    char path[256]; memset(path, 0, 256);
    int success = initialize_connection(WRITE_PIPE_NAME, READ_PIPE_NAME, &WRITE_PIPE, &READ_PIPE);
    if(success == ERROR) return ERROR;

    do {
        unsigned char lungime = 0;
        char content[250]; memset(content, 0, 250);
        EVALUATE_BREAK((success = read_from_pipe(READ_PIPE, content, sizeof(content), &lungime)), SUCCESS);

        if(strcmp(content, "VARIANT\0") == 0) {
            STRING_FIELD request;
            strcpy(request.contents, "VARIANT\0");  request.size = strlen(request.contents);

            STRING_FIELD parameter;
            strcpy(parameter.contents, "VALUE\0");  parameter.size = strlen(parameter.contents);

            unsigned int numberField = VARIANT_NUMBER;
            EVALUATE_BREAK(send_over_pipe(WRITE_PIPE, request),    SUCCESS);
            EVALUATE_BREAK(write(WRITE_PIPE, &numberField, sizeof(unsigned int)), sizeof(unsigned int));
            EVALUATE_BREAK(send_over_pipe(WRITE_PIPE, parameter),  SUCCESS);
        } 
        
        else if(strcmp(content, "CREATE_SHM\0") == 0) {
            STRING_FIELD request;
            strcpy(request.contents, "CREATE_SHM\0");  request.size = strlen(request.contents);
            EVALUATE_BREAK(send_over_pipe(WRITE_PIPE,  request),     SUCCESS);

            unsigned int number = -1; read(READ_PIPE, &number, sizeof(number));

            STRING_FIELD status; 
            if((SHARED_MEMORY = create_shm()) == ERROR) strcpy(status.contents, "ERROR\0");
            else strcpy(status.contents, "SUCCESS\0");

            status.size = strlen(status.contents);
            EVALUATE_BREAK(send_over_pipe(WRITE_PIPE, status), SUCCESS);
            continue;
        } 
        
        else if(strcmp(content, "WRITE_TO_SHM\0") == 0) {
            STRING_FIELD requestType;
            ASSIGN_STRING_FIELD(requestType, "WRITE_TO_SHM\0");
            EVALUATE_BREAK(send_over_pipe(WRITE_PIPE, requestType), SUCCESS);

            unsigned int OFFSET = 0;
            unsigned int VALUE  = 0;

            EVALUATE_BREAK(read(READ_PIPE, &OFFSET, sizeof(unsigned int)), sizeof(unsigned int));
            EVALUATE_BREAK(read(READ_PIPE, &VALUE,  sizeof(unsigned int)), sizeof(unsigned int));

            STRING_FIELD status;
            if(OFFSET < 0 || OFFSET > SHM_SIZE || OFFSET + 4 > SHM_SIZE) {
                ASSIGN_STRING_FIELD(status, "ERROR\0"); 
            } else {
                if(write_in_memory(OFFSET, VALUE, SHARED_MEMORY) == SUCCESS) {
                    ASSIGN_STRING_FIELD(status, "SUCCESS\0");
                } else ASSIGN_STRING_FIELD(status, "ERROR\0");
            }

            EVALUATE_BREAK(send_over_pipe(WRITE_PIPE, status),  SUCCESS);
        }

        else if(strcmp(content, "MAP_FILE\0") == 0) {
            STRING_FIELD requestType;
            ASSIGN_STRING_FIELD(requestType, "MAP_FILE\0");
            EVALUATE_BREAK(send_over_pipe(WRITE_PIPE, requestType), SUCCESS);

            EVALUATE_BREAK(read(READ_PIPE, &lungime, sizeof(char)), sizeof(char));
            EVALUATE_BREAK(read(READ_PIPE, path,  (lungime) * sizeof(unsigned char)), (lungime)  * sizeof(unsigned char));
            path[lungime + 1] = 0;

            STRING_FIELD status;
            if(put_file_in_shm(path, &FILE_DESCRIPTOR) == ERROR) {
                ASSIGN_STRING_FIELD(status, "ERROR\0");
            } else ASSIGN_STRING_FIELD(status, "SUCCESS\0");
           
            EVALUATE_BREAK(send_over_pipe(WRITE_PIPE, status), SUCCESS);
        } 

        else if(strcmp(content, "READ_FROM_FILE_OFFSET\0") == 0) {
            unsigned int OFFSET = 0;
            unsigned int NUMBER_OF_BYTES = 0;

            EVALUATE_BREAK(read(READ_PIPE, &OFFSET,           sizeof(unsigned int)), sizeof(unsigned int));
            EVALUATE_BREAK(read(READ_PIPE, &NUMBER_OF_BYTES,  sizeof(unsigned int)), sizeof(unsigned int));

            STRING_FIELD requestType;
            ASSIGN_STRING_FIELD(requestType, "READ_FROM_FILE_OFFSET\0");
            EVALUATE_BREAK(send_over_pipe(WRITE_PIPE, requestType), SUCCESS);

            STRING_FIELD status;
            if(read_bytes_from_offset(OFFSET, NUMBER_OF_BYTES, FILE_DESCRIPTOR) == SUCCESS) {
                ASSIGN_STRING_FIELD(status, "SUCCESS\0");
            } else ASSIGN_STRING_FIELD(status, "ERROR\0");

            EVALUATE_BREAK(send_over_pipe(WRITE_PIPE, status), SUCCESS);
        } 
        
        else if(strcmp(content, "READ_FROM_FILE_SECTION\0") == 0) {
            STRING_FIELD requestType;
            ASSIGN_STRING_FIELD(requestType, "READ_FROM_FILE_SECTION\0");
            EVALUATE_BREAK(send_over_pipe(WRITE_PIPE, requestType), SUCCESS);

            unsigned int SECTION_NUMBER, OFFSET, NUMBER_OF_BYTES;
            EVALUATE_BREAK(read(READ_PIPE, &SECTION_NUMBER,   sizeof(unsigned int)), sizeof(unsigned int));
            EVALUATE_BREAK(read(READ_PIPE, &OFFSET,           sizeof(unsigned int)), sizeof(unsigned int));
            EVALUATE_BREAK(read(READ_PIPE, &NUMBER_OF_BYTES,  sizeof(unsigned int)), sizeof(unsigned int));

            HEADER header;
            STRING_FIELD status;
            if(parse(FILE_DESCRIPTOR, FALSE, &header) == ERROR) {
                ASSIGN_STRING_FIELD(status, "ERROR\0");                
            } else {
                if(read_from_file_header(header, OFFSET, SECTION_NUMBER, NUMBER_OF_BYTES, FILE_DESCRIPTOR) == SUCCESS) {
                    ASSIGN_STRING_FIELD(status, "SUCCESS\0");
                } else ASSIGN_STRING_FIELD(status, "ERROR\0");  
            }
            EVALUATE_BREAK(send_over_pipe(WRITE_PIPE, status), SUCCESS);
        } 
        
          
        else if(strcmp(content, "READ_FROM_LOGICAL_SPACE_OFFSET\0") == 0) {
            STRING_FIELD requestType;
            ASSIGN_STRING_FIELD(requestType, "READ_FROM_LOGICAL_SPACE_OFFSET\0");
            EVALUATE_BREAK(send_over_pipe(WRITE_PIPE, requestType), SUCCESS);

            unsigned int OFFSET, NUMBER_OF_BYTES;
            EVALUATE_BREAK(read(READ_PIPE, &OFFSET,            sizeof(unsigned int)), sizeof(unsigned int));
            EVALUATE_BREAK(read(READ_PIPE, &NUMBER_OF_BYTES,   sizeof(unsigned int)), sizeof(unsigned int));

            HEADER header;
            STRING_FIELD status;
            if(parse(FILE_DESCRIPTOR, FALSE, &header) == ERROR) {
                ASSIGN_STRING_FIELD(status, "ERROR\0");                
            }  
            if(read_from_logical_space_offset(header, OFFSET, NUMBER_OF_BYTES, FILE_DESCRIPTOR) == SUCCESS) {
                ASSIGN_STRING_FIELD(status, "SUCCESS\0");
            } else ASSIGN_STRING_FIELD(status, "ERROR\0");  
            EVALUATE_BREAK(send_over_pipe(WRITE_PIPE, status), SUCCESS);

        } else break;
        
    } while(!FINISHED);


    close_pipe(READ_PIPE, READ_PIPE_NAME);
    close_pipe(WRITE_PIPE, WRITE_PIPE_NAME);
    return SUCCESS;
}

void print_header_details(HEADER* current_header) {
    if(current_header == NULL) return;
    printf("version=%d\n",     current_header->version);
    printf("nr_sections=%d\n", current_header->number_of_sections);
    for(int i = 0; i<current_header->number_of_sections; i++) {
        printf("section%d: section name = %s | section type = %d | section size = %d | section offset = %d\n", 
            i+1, 
            current_header->sections[i].section_name,
            current_header->sections[i].sec_type,
            current_header->sections[i].section_size,
            current_header->sections[i].section_offset
        );
    }
}

int create_pipe(char* path) {
    int fd  = -1;
    if((fd = mkfifo(path, 0600)) != 0) {
        perror("Could not create the PIPE");
        return ERROR;
    }
    return fd;
}

int open_pipe(char* path, int MODE) {
    int file_descriptor = -1;

    file_descriptor = open(path, MODE);
    if(file_descriptor == -1) {
        perror("Could not open the pipe.");
        return ERROR;
    }
    return file_descriptor;
}

void close_pipe(int fd, char* path) {
    close(fd);
    unlink(path);
}

STRING_FIELD* init_string_field(unsigned char size, char* contents) {
    int status = 0;
    STRING_FIELD* new_field = NULL;

    do{
        if(size > 250) {
            printf("ERR: Can't create a the new field. The size %d is bigger than 250.\n", size);
            status = -1;
            break;
        }

        new_field = (STRING_FIELD*)malloc(sizeof(STRING_FIELD));
        if(new_field == NULL) {
            status = -1;
            break;
        }

        memset(new_field, 0, sizeof(STRING_FIELD));

        new_field->size = size;
        if(strlen(contents) > 250) {
            printf("ERR: Can't create a the new contents field.");
            status = -1;
            break;
        }
        
        memset(new_field->contents, 0, 256);
        strcpy(new_field->contents, contents);

    } while(FALSE);

    if(status == -1) free_string_field(new_field);
    return new_field;
}

void free_string_field(STRING_FIELD* field) {
    if(field == NULL) return;
    free(field);   field = NULL;
}

void display_string_field(STRING_FIELD* field) {
    if(field == NULL) return;
    printf("STRING FIELD %p's data:\n\tcapacity: %d\n\tcontents:\n\t\tcontent message: %s\n\t\tactual content: ", field, (int)field->size, field->contents);
    
    printf("%x ", field->size);
    for(int i = 0; i<field->size; i++) printf("%x ", field->contents[i]);
    printf("\n");
}

int send_over_pipe(int PIPE, STRING_FIELD field) {
    do{
        EVALUATE_BREAK(write(PIPE, &field.size,     sizeof(field.size)), sizeof(field.size));
        EVALUATE_BREAK(write(PIPE, field.contents,  field.size), field.size);
        return SUCCESS;
    } while(TRUE);
    return ERROR;
}

int read_from_pipe(int fd, char *buffer, unsigned short buffer_size, unsigned char *len_out) {
    do{
        *len_out = 0;
        if(buffer_size > 250) {
            printf("[READ_FROM_PIPE]: Buffer size is invalid.");
            break;
        }

        EVALUATE_BREAK(read(fd, len_out, sizeof(unsigned char)), sizeof(unsigned char));
        EVALUATE_BREAK(read(fd, buffer,  *len_out * sizeof(char)), *len_out * sizeof(char));

        return SUCCESS;
    } while(TRUE);

    return ERROR;
}

int initialize_connection(char* write_pipe, char* read_pipe, int* write_pipe_descriptor, int* read_pipe_descriptor) {

    STRING_FIELD* test_connection = init_string_field(LENGTH_AND_STRING(INIT_CONNECTION_STRING_FIELD));

    if(test_connection == NULL) {
        perror("Can't init the string field: ");
        return ERROR;
    }
    
    do {
        int CREATE_WRITE_PIPE = create_pipe(write_pipe);
        if(CREATE_WRITE_PIPE == ERROR) {
            printf("ERROR\n cannot create the response pipe\n");
            break;
        }

        int READ_PIPE  = open_pipe(read_pipe,  O_RDONLY);
        if(READ_PIPE == ERROR) {
            printf("ERROR\n cannot open the request pipe\n");
            break;
        }

        int WRITE_PIPE = open_pipe(write_pipe, O_WRONLY);
        if(READ_PIPE == ERROR) {
            printf("ERROR\n cannot open the response pipe\n");
            break;
        }

        if(send_over_pipe(WRITE_PIPE, *test_connection) == SUCCESS) {
            *write_pipe_descriptor = WRITE_PIPE;
            *read_pipe_descriptor  = READ_PIPE;
            return SUCCESS;
        }
    } while(TRUE);

    return ERROR;
}

int create_shm() {

    do {
        int SHARED_MEMORY = shm_open(SHM_NAME, O_CREAT | O_RDWR, SHM_PERMS);
        if(SHARED_MEMORY < 0) {
            perror("Could not create the shared memory.");
            break;
        }

        ftruncate(SHARED_MEMORY, SHM_SIZE);
        SHARED_MEMORY_ADDRESS = mmap(NULL, SHM_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, SHARED_MEMORY, 0);
        return SHARED_MEMORY;
    } while(TRUE);

    return ERROR;
}

int write_in_memory(unsigned int offset, unsigned int value, int shared_memory) {
    do {
        volatile char* address = NULL;
        EVALUATE_ERROR((address = (volatile char*)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory, 0)), MAP_FAILED);

        *(volatile unsigned int*)(address + offset) = value;
        EVALUATE_BREAK(munmap((void*)address, SHM_SIZE), SUCCESS);
        return SUCCESS;
    } while(TRUE);

    return ERROR;
}

int put_file_in_shm(char* path, int* file_descriptor) {

    do {
        *file_descriptor = -1;
        EVALUATE_ERROR((*file_descriptor = open(path, O_RDONLY)), ERROR);

        int size = lseek(*file_descriptor, 0, SEEK_END);
        lseek(*file_descriptor, 0, SEEK_SET);

        char* address = NULL;
        EVALUATE_ERROR((address = (char*)mmap(NULL, size, PROT_READ, MAP_SHARED, *file_descriptor, 0)), MAP_FAILED);
        
        return SUCCESS;
    } while(TRUE);

    return ERROR;
}

int read_bytes_from_offset(unsigned int OFFSET, unsigned int NUMBER_OF_BYTES, int FILE_DESCRIPTOR) {
    do {
        int size = lseek(FILE_DESCRIPTOR, 0, SEEK_END);
        EVALUATE_BREAK((OFFSET + NUMBER_OF_BYTES > size), FALSE);
        lseek(FILE_DESCRIPTOR, 0, SEEK_SET);

        char* address = NULL;
        EVALUATE_ERROR((address = (char*)mmap(NULL, size, PROT_READ, MAP_SHARED, FILE_DESCRIPTOR, 0)), MAP_FAILED);
        memcpy(SHARED_MEMORY_ADDRESS, address + OFFSET, NUMBER_OF_BYTES);
        return SUCCESS;
    } while(TRUE);

    return ERROR;
}

void free_header_data(HEADER* header) {
    if(header == NULL) return;

    if(header->magic != NULL) {
        free(header->magic);    
        header->magic = NULL;
    }

    free(header); header = NULL;
}


void free_section_header_data(HEADER_SECTION* header_section) {
    if(header_section == NULL) return; 
    free(header_section); header_section = NULL;
}

int parse(int FILE_DESCRIPTOR, int silent, HEADER* out) {

   do {
        int size = lseek(FILE_DESCRIPTOR, 0, SEEK_END);
        lseek(FILE_DESCRIPTOR, 0, SEEK_SET);

        char* address = NULL;
        EVALUATE_ERROR((address = (char*)mmap(NULL, size, PROT_READ, MAP_SHARED, FILE_DESCRIPTOR, 0)), MAP_FAILED);
        EVALUATE_BREAK((*((unsigned char*)(address + size -1)) == 'G' && *((unsigned char*)(address + size - 2)) == 'M'), TRUE);


        out->header_size         = *((short*)(address + size - 4));
        address = address + size - out->header_size;

        out->version             = *((short*)(address));
        address = address + 2;

        out->number_of_sections  = *((unsigned char*)(address));
        address = address + 1;
 
        for(int i = 0; i<out->number_of_sections; i++) {           
            strncpy(
                out->sections[i].section_name, 
                (char*)(address), 
                SECTION_NAME_SIZE
            );  

            address = address + SECTION_NAME_SIZE;

            out->sections[i].sec_type       = *((unsigned char*)(address)); address = address + 1;
            out->sections[i].section_offset = *((unsigned int*)(address));  address = address + 4;
            out->sections[i].section_size   = *((unsigned int*)(address));  address = address + 4;
        }
        return SUCCESS;
   } while(FALSE);

   return ERROR;
}

int read_from_file_header(HEADER header, unsigned int OFFSET, unsigned int SECTION_NUMBER, unsigned int NUMBER_OF_BYTES, int FILE_DESCRIPTOR) {
    
    do{
        int size = lseek(FILE_DESCRIPTOR, 0, SEEK_END);
        lseek(FILE_DESCRIPTOR, 0, SEEK_SET);

        EVALUATE_BREAK((SECTION_NUMBER < 1 || SECTION_NUMBER > header.number_of_sections), FALSE);

        unsigned int SECTION_OFFSET = header.sections[SECTION_NUMBER-1].section_offset;
        unsigned int SECTOR_OFFSET  = SECTION_OFFSET + OFFSET;

        char* address = NULL;
        EVALUATE_ERROR((address = (char*)mmap(NULL, size, PROT_READ, MAP_SHARED, FILE_DESCRIPTOR, 0)), MAP_FAILED);
        memcpy(SHARED_MEMORY_ADDRESS, address + SECTOR_OFFSET, NUMBER_OF_BYTES);
        return SUCCESS;
    } while(TRUE);

    return ERROR;
}

/// multumesc tuturor colegilor care si-au dat sufletul explicandu-mi ce trebuie sa fac la acest task
/// inca nu sunt sigur ca am inteles 100%
/// gracias @emeric, @diana, @dani si @grigo
int read_from_logical_space_offset(HEADER header, unsigned int OFFSET, unsigned int NUMBER_OF_BYTES, int FILE_DESCRIPTOR) {
    do {
        int size = lseek(FILE_DESCRIPTOR, 0, SEEK_END);
        lseek(FILE_DESCRIPTOR, 0, SEEK_SET);

        char* address = NULL;
        EVALUATE_ERROR((address = (char*)mmap(0,size, PROT_READ, MAP_SHARED, FILE_DESCRIPTOR, 0)), MAP_FAILED);

        unsigned int FINAL_OFFSET = 0;
        int ok = 0;
        
        for(int i = 0, PAGE_OFFSET = 0; i<header.number_of_sections; i++) {
            if(PAGE_OFFSET + header.sections[i].section_size > OFFSET) {
                FINAL_OFFSET = OFFSET - PAGE_OFFSET + header.sections[i].section_offset;
                ok = 1;
                break;
            }

            PAGE_OFFSET += (header.sections[i].section_size / LOGICAL_OFFSET_PAGE_SIZE + 1) * LOGICAL_OFFSET_PAGE_SIZE;
        }

        EVALUATE_BREAK(ok, TRUE);
        memcpy(SHARED_MEMORY_ADDRESS, address + FINAL_OFFSET, NUMBER_OF_BYTES);

        return SUCCESS;
    } while(TRUE);

    return ERROR;
}