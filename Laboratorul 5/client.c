#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char** argv) {

    printf("%c ", '>');

    int first_operand, second_operand;
    char operator;
    scanf("%d %c %d", &first_operand, &operator, &second_operand);

    char* first_operand_string  = (char*)calloc(6, sizeof(char));    sprintf(first_operand_string, "%d", first_operand);
    char* operator_string       = (char*)calloc(1, sizeof(char));    sprintf(operator_string, "%c", operator);
    char* second_operand_string = (char*)calloc(6, sizeof(char));    sprintf(second_operand_string, "%d", second_operand);

    execl("server", "server", first_operand_string, operator_string, second_operand_string, NULL);
    return 0;
}