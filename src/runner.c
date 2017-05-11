#include "runner.h"
#include "field.h"
#include "life.h"

#include <semaphore.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

struct _barrier_struct {
    sem_t *semaphore;
    pthread_mutex_t *mutex;
    uint maximumValue;
    uint currentValue;
};

Barrier* create_barrier(uint maximumValue) {
    Barrier *result = (Barrier*)malloc(sizeof(Barrier));

    result->semaphore = (sem_t*)malloc(sizeof(sem_t));
    sem_init(result->semaphore, 0, 0);

    result->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(result->mutex, NULL);

    result->maximumValue = maximumValue;
    result->currentValue = 0;

    return result;
}

void destroy_barrier(Barrier *barrier) {
    sem_destroy(barrier->semaphore);
    pthread_mutex_destroy(barrier->mutex);
    free(barrier);
}

void barrier_wait(Barrier *barrier) {
    pthread_mutex_lock(barrier->mutex);

    ++barrier->currentValue;

    if (barrier->currentValue == barrier->maximumValue) {
        barrier->currentValue = 0;

        for (uint count = 1; count < barrier->maximumValue; ++count) {
            sem_post(barrier->semaphore);
        }

        pthread_mutex_unlock(barrier->mutex);
        return;
    }

    pthread_mutex_unlock(barrier->mutex);

    sem_wait(barrier->semaphore);
}

Runner* create_empty_runner(uint width, uint height, uint turnsCount, uint workersCount) {
    Runner *result = (Runner*)malloc(sizeof(Runner));

    result->turnsCount = turnsCount;
    result->currentTurn = 0;

    result->field = create_empty_field(width, height);
    result->tmpField = create_empty_field(width, height);

    result->workersCount = workersCount;
    result->workers = (pthread_t*)malloc(sizeof(pthread_t) * workersCount);

    result->startBarrier = create_barrier(workersCount + 1);
    result->finishBarrier = create_barrier(workersCount + 1);

    return result;
}

Runner* create_random_runner(uint width, uint height, uint turnsCount, uint workersCount) {
    Runner *result = create_empty_runner(width, height, turnsCount, workersCount);

    result->field = create_random_configuration(width, height);

    return result;
}

Runner* create_glider_test_runner(uint width, uint height, uint turnsCount, uint workersCount) {
    Runner *result = create_empty_runner(width, height, turnsCount, workersCount);

    set_cell(result->field, 1, 0, 1);
    set_cell(result->field, 2, 1, 1);
    set_cell(result->field, 2, 2, 1);
    set_cell(result->field, 1, 2, 1);
    set_cell(result->field, 0, 2, 1);

    return result;
}

void destroy_runner(Runner *runner) {
    destroy_field(runner->field);
    destroy_field(runner->tmpField);

    free(runner->workers);

    destroy_barrier(runner->startBarrier);
    destroy_barrier(runner->finishBarrier);

    free(runner);
}

typedef struct _simulation_arguments_struct {
    uint lowerBound;
    uint upperBound;
    Runner *runner;
    uint turnsCount;
} SimulationArguments;

void* simulate_chunk(void *arguments) {
    SimulationArguments *args = (SimulationArguments*)arguments;
    while (args->turnsCount--) {
        barrier_wait(args->runner->startBarrier);

        simulate_step(args->runner->field, args->runner->tmpField, args->lowerBound, args->upperBound);

        barrier_wait(args->runner->finishBarrier);
    }

    return NULL;
}

void swap_fields(Runner *runner) {
    Field *temporary = runner->field;
    runner->field = runner->tmpField;
    runner->tmpField = temporary;
}

void run(Runner *runner) {
    SimulationArguments *arguments = malloc(sizeof(SimulationArguments) * runner->workersCount);
    uint chunkSize = runner->field->width * runner->field->height / runner->workersCount;

    assert(runner->workersCount);

    for (uint index = 0; index < runner->workersCount; ++index) {
        if (index == runner->workersCount - 1) {
            //If area of field is not divisible by number of workers, the last worker must take all remaining cells
            arguments[index].upperBound = runner->field->width * runner->field->height;
            arguments[index].lowerBound = (runner->workersCount - 1) * chunkSize;
        } else {
            arguments[index].lowerBound = index * chunkSize;
            arguments[index].upperBound = (index + 1) * chunkSize;
        }

        arguments[index].runner = runner;
        arguments[index].turnsCount = runner->turnsCount;
        pthread_create(&runner->workers[index], 0, simulate_chunk, &arguments[index]);
    }

    while (runner->currentTurn < runner->turnsCount) {
        barrier_wait(runner->startBarrier);

        ++runner->currentTurn;

        barrier_wait(runner->finishBarrier);

        swap_fields(runner);
    }

    for (uint index = 0; index < runner->workersCount; ++index) {
        pthread_join(runner->workers[index], 0);
    }
}

void print_state(const Runner *runner) {
    printf("Turn %d:\n", runner->currentTurn);

    for (uint yPos = 0; yPos < runner->field->height; ++yPos) {
        for (uint xPos = 0; xPos < runner->field->width; ++xPos) {
            if (get_cell(runner->field, xPos, yPos) == CS_ALIVE) {
                printf("X");
            } else {
                printf(".");
            }
        }
        puts("");
    }
}
