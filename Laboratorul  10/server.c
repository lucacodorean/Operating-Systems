#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_NAME     "l11_my_fifo_server_client"
#define FIFO_NAME_out "l11_my_fifo_server_client_returned"

int main(void)
{

    int fd = -1;
    int fd2 = -1;



    //open, write and close fifo
    fd = open(FIFO_NAME, O_RDONLY);
    if(fd == -1) {
        perror("Could not open FIFO for writing");
        return 1;
    }

    fd2 = open(FIFO_NAME_out, O_WRONLY);

    char x, sign, y;

    int rezultat = 0;

    if(x != '\0') {
        do {
            read(fd, &x, 1); read(fd, &sign, 1); read(fd, &y, 1);
            printf("%c %c %c", x, sign, y);

            if(sign == '+') rezultat = (int)x + (int)y;
            else if(sign == '-') rezultat = (int)x - (int)y;

            read(fd2, &rezultat, sizeof(int));

        } while(x != '\0');
    }
    close(fd);

    //delete fifo
    unlink(FIFO_NAME);
    unlink(FIFO_NAME_out);

    return 0;
}