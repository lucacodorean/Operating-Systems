#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define NR_THREADS 7
#define AMOUNT_DEPOSIT 11
#define AMOUNT_WITHDRAW 7

typedef struct {
    int id;
    pthread_mutex_t *lock;
    pthread_cond_t *cond;

    pthread_mutex_t *lock_sleep;
    pthread_cond_t *cond_sleep;
} TH_STRUCT;


int balance = 0, TURN = 0;

void *thread_withdraw(void *arg)
{
    TH_STRUCT *s = (TH_STRUCT*)arg;
    
    pthread_mutex_lock(s->lock_sleep);
    TURN++;
    pthread_cond_signal(s->cond_sleep);
    while(TURN != s->id) {
        pthread_cond_wait(s->cond_sleep, s->lock_sleep);
    }
    pthread_mutex_unlock(s->lock_sleep);

    pthread_mutex_lock(s->lock);

    while(balance < AMOUNT_WITHDRAW) {
        printf("[Th%d] Not enough money (%d). Will wait...\n", s->id, balance);
        pthread_cond_wait(s->cond, s->lock);
    }
    balance -= AMOUNT_WITHDRAW;
    printf("[Th%d] Withdrawn %d. Current balance is %d.\n", 
            s->id, AMOUNT_WITHDRAW, balance);
    pthread_mutex_unlock(s->lock);

    pthread_mutex_lock(s->lock_sleep);
    TURN++;
    pthread_cond_broadcast(s->cond_sleep);
    pthread_mutex_unlock(s->lock_sleep);

    return NULL;
}

void *thread_deposit(void *arg)
{
    TH_STRUCT *s = (TH_STRUCT*)arg;

    pthread_mutex_lock(s->lock_sleep);
    while(TURN != s->id) {
        pthread_cond_wait(s->cond_sleep, s->lock_sleep);
    }
    pthread_mutex_unlock(s->lock_sleep);


    pthread_mutex_lock(s->lock);
    balance += AMOUNT_DEPOSIT;
    printf("[Th%d] Deposited %d. Current balance is %d.\n", 
            s->id, AMOUNT_DEPOSIT, balance);

    pthread_mutex_lock(s->lock_sleep);
    TURN++;
    pthread_cond_broadcast(s->cond_sleep);
    pthread_mutex_unlock(s->lock_sleep);

    if(balance >= 2 * AMOUNT_WITHDRAW) {
        pthread_cond_broadcast(s->cond);
    } else if (balance >= AMOUNT_WITHDRAW) {
        pthread_cond_signal(s->cond);
    }
    pthread_mutex_unlock(s->lock);



    return NULL;
}

int main()
{
    int i;
    TH_STRUCT params[NR_THREADS];
    pthread_t tids[NR_THREADS];
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t lock_sleep = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t  cond_sleep = PTHREAD_COND_INITIALIZER;
    
    for(i=0; i<NR_THREADS; i++) {
        params[i].id = i;
        params[i].lock = &lock;
        params[i].cond = &cond;
        params[i].lock_sleep = & lock_sleep;
        params[i].cond_sleep = & cond_sleep;
        if(i == 0 || i == 2 || i == 3 || i == 4) {
            pthread_create(&tids[i], NULL, thread_withdraw, &params[i]);
        } else {
            pthread_create(&tids[i], NULL, thread_deposit, &params[i]);
        }
        
    }

    pthread_cond_signal(&cond_sleep);

    for(i=0; i<NR_THREADS; i++) {
        pthread_join(tids[i], NULL);
    }
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&lock);

    return 0;
}