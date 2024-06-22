#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_NAME     "l11_my_fifo_server_client"
#define FIFO_NAME_out "l11_my_fifo_server_client_returned"

int main(void)
{
    int fd = -1, fd2 = -1;
    //create fifo
    if(mkfifo(FIFO_NAME, 0600) != 0) {
        perror("Could not create pipe");
        return 1;
    }

    //open, write and close fifo
    fd2 = open(FIFO_NAME_out, O_RDONLY);
    if(fd2 == -1) {
        perror("Could not open FIFO for writing");
        return 1;
    }
    
    //open, write and close fifo
    fd = open(FIFO_NAME, O_WRONLY);
    if(fd == -1) {
        perror("Could not open FIFO for writing");
        return 1;
    }
    

    int rezultat = 0;
    char x,sign,y;
    while(x != EOF) {
        scanf("%c%c%c", &x, &sign, &y);
        write(fd, &x, 1); write(fd, &sign, 1); write(fd, &y, 1);
        printf("Sent to parent: %c%c%c", x, sign, y);

        read(fd2, &rezultat, sizeof(int));
        printf("\nClient received from server: %d", rezultat);
    }



    close(fd);

    //delete fifo
    unlink(FIFO_NAME);

    return 0;
}