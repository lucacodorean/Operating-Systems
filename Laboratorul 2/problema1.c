/**
Generate all the subsets of the set {0, 1, 2, ..., N-1}
using as little memory as possible. How many subsets are there?

Example: for N=3, the program will output:
"{}, {0 }, {1 }, {0 1 }, {2 }, {0 2 }, {1 2 }, {0 1 2 }, 
Exista 8 submultimi"
*/

/**
Sa se genereze toate submultimile multimii {0, 1, 2, ..., N-1}
folosind cat mai putina memorie. Cate submultimi exista?

Exemplu: pentru N=3, programul va afisa:
"{}, {0 }, {1 }, {0 1 }, {2 }, {0 2 }, {1 2 }, {0 1 2 }, 
Exista 8 submultimi"
*/

#include <stdio.h>
#include <stdlib.h>

#define ISBITSET(number, POS) ((number)&(1 << POS))

int generateSets(int largest_num)
{
    int i, j;
    int maxLoop;

    maxLoop = 1 << largest_num;

    for (i = 0; i < maxLoop; i++) {
        printf("{");
        for (j = 0; j < largest_num; j++){
            if (ISBITSET(i, j) != 0) {
                printf("%d ", j);
            }
        }
        printf("}, ");
    }
    return maxLoop; 
}

int main(int argc, char **argv)
{
    int largest_num;
    int res;

    if(2 != argc){
        printf("USAGE: %s number\n", argv[0]);
        exit(-1);
    }

    sscanf(argv[1], "%d", &largest_num);
    res = generateSets(largest_num);
    printf("\nExista %d submultimi\n", res);
    return 0;
}