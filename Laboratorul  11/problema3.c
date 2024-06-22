#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
    int fd;
    off_t size, i;
    char *data = NULL;
    
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        return 1;
    }

    fd = open(argv[1], O_RDWR);
    if(fd == -1) {
        perror("Could not open input file");
        return 1;
    }
    
    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    data = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(data == (void*)-1) {
        perror("Could not map file");
        close(fd);
        return 1;
    }

    int location = strlen(data) + 1;
    int read_index = 0, write_index = 0;

    for(int i = 0; i<location; i++) {
        if(strchr("aeiou", data[i]) != 0) {
            read_index++;
        } else {
            data[write_index] = data[read_index];
            read_index++;
            write_index++;
        }
    }

    for(int i = read_index - write_index; i<location; i++) data[i] = ' ';

    munmap(data, size);
    data = NULL;
    close(fd);

    return 0;
}