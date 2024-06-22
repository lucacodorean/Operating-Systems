#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv) {

    if(argc != 4) {
        printf("Server: parameter error");
        return 1;
    }

    int first_operator  = atoi(argv[1]);
    int operator        = (strcmp(argv[2], "+") == 0 ? (int)'+' : (int)'-');
    int second_operator = atoi(argv[3]);

    switch (operator)
    {
        case (int)'+':
            printf("%d + %d = %d", first_operator, second_operator, first_operator + second_operator);
            break;

        case (int)'-':
            printf("%d - %d = %d", first_operator, second_operator, first_operator - second_operator);
            break;

        default:
            printf("Server: invalid operation.");
            break;
    }

    return 0;
}