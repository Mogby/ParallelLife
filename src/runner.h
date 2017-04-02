#ifndef RUNNER_H

#define RUNNER_H

#include <pthread.h>
#include <semaphore.h>

typedef unsigned int uint;

typedef struct _field_struct Field;

typedef struct _runner_struct {
    uint turnsCount;
    uint currentTurn;
    uint mult; 
    Field *field;
    Field *tmpField;

    uint workersCount;
    pthread_t *workers;

    pthread_barrier_t start;
    pthread_barrier_t finish;
} Runner;

Runner* create_empty_runner(uint width, uint height, uint turnsCount, uint workersCount, uint mult);

Runner* create_random_runner(uint width, uint height, uint turnsCount, uint workersCount, uint mult);

Runner* create_glider_test_runner(uint width, uint height, uint turnsCount, uint workersCount, uint mult);
void destroy_runner(Runner *runner);

void run(Runner *runner);

void print_state(const Runner *runner);

void print_state_nc(const Runner *runner);

#endif
