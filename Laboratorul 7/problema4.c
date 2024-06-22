#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>

void* thread_function_creator(void*);
int   nrThreadsNa = 0;
int   nrThreadsCl = 0;

bool done = false;

#define Na 0
#define Cl 1

typedef struct {
    bool type;
    pthread_t id;
} THREAD_STRUCTURE_CREATOR;

pthread_t ids[1000];

int main(void) {

    srand(time(NULL));

    THREAD_STRUCTURE_CREATOR data[1000];
    int l = 0;

    while(!done)                pthread_create(&ids[l], NULL, thread_function_creator, &data[l++]);

    for(size_t i = 0; i<1000; i++)  data[i].id = ids[i];
    for(size_t i = 0; i<l;    i++)  pthread_join(ids[i], NULL);

    return 0;
}

void* thread_function_creator(void* args) {
    THREAD_STRUCTURE_CREATOR* actual_value = (THREAD_STRUCTURE_CREATOR*)args;
    actual_value->type = rand() % 2;

    printf("S-a generat o molecula de %s\n", actual_value->type == Na ? "Na" : "Cl");

    int current_Na = nrThreadsNa, current_Cl = nrThreadsCl;

    if(actual_value->type == Na) nrThreadsNa++;
    else nrThreadsCl++;

    if(current_Na != nrThreadsNa && actual_value->type == Na) {
        if(nrThreadsCl != 0) {
            printf("Na generated: S-a realizat o molecula de NaCl cu moleculele cu id: %ld %ld\n", actual_value->id, ids[nrThreadsCl]);
            nrThreadsCl--;
            nrThreadsNa--;
            done = true;
        }
    } 

    else if(current_Cl != nrThreadsCl && actual_value->type == Cl) {
        if(nrThreadsNa != 0) {
            printf("Cl generated: S-a realizat o molecula de NaCl cu moleculele cu id: %ld %ld\n", actual_value->id, ids[nrThreadsNa]);
            nrThreadsCl--;
            nrThreadsNa--;
            done = true;
        }
    } 

    return NULL;
}
