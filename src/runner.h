#ifndef RUNNER_H

#define RUNNER_H

#include <semaphore.h>

typedef unsigned int uint;

typedef struct _field_struct Field;

typedef struct _runner_struct {
    uint turnsCount;
    uint currentTurn;

    Field *field;
    Field *tmpField;

    uint workersCount;
    pid_t *workers;

    sem_t **startSem;
    sem_t **middleSem;
    sem_t **finishSem;
} Runner;

Runner* create_empty_runner(uint width, uint height, uint turnsCount, uint workersCount);

Runner* create_random_runner(uint width, uint height, uint turnsCount, uint workersCount);

Runner* create_glider_test_runner(uint width, uint height, uint turnsCount, uint workersCount);

int run(Runner *runner);

void print_state(const Runner *runner);

#endif
