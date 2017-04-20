#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "runner.h"
#include "field.h"
#include "life.h"

Runner* create_empty_runner(uint width, uint height, uint turnsCount, uint workersCount) {
    Runner *result = (Runner*)malloc(sizeof(Runner));

    result->turnsCount = turnsCount;
    result->currentTurn = 0;

    result->field = create_empty_field(width, height);
    result->tmpField = create_empty_field(width, height);

    result->workersCount = workersCount;

    result->workers = (pthread_t*)malloc(sizeof(pthread_t) * workersCount);

    result->startSem = (sem_t*)malloc(sizeof(sem_t) * workersCount);
    result->middleSem = (sem_t*)malloc(sizeof(sem_t) * workersCount);
    result->finishSem = (sem_t*)malloc(sizeof(sem_t) * workersCount);

    for (uint index = 0; index < workersCount; ++index) {
        sem_init(result->startSem + index, 0, 0);
        sem_init(result->middleSem + index, 0, 0);
        sem_init(result->finishSem + index, 0, 0);
    }

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

    for (uint index = 0; index < runner->workersCount; ++index) {
        sem_destroy(runner->startSem + index);
        sem_destroy(runner->middleSem + index);
        sem_destroy(runner->finishSem + index);
    }

    free(runner);
}

void copy_runner(void *dest, Field *field) {
    memcpy(dest, (void*)field, sizeof(Field));
    dest = (void*)((char*)dest + sizeof(Field));
}

typedef struct _simulation_arguments_struct {
    uint lowerBound;
    uint upperBound;
    uint workerId;
    Runner *runner;
} SimulationArguments;

void* simulate_chunk(void *arguments) {
    SimulationArguments *args = (SimulationArguments*)arguments;
    while (args->runner->currentTurn < args->runner->turnsCount) {
        sem_post(args->runner->startSem + args->workerId);

        simulate_step(args->runner->field, args->runner->tmpField, args->lowerBound, args->upperBound);

        sem_post(args->runner->middleSem + args->workerId);

        sem_wait(args->runner->finishSem + args->workerId);
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
            arguments[runner->workersCount - 1].lowerBound = (runner->workersCount - 1) * chunkSize;
            arguments[runner->workersCount - 1].upperBound = runner->field->width * runner->field->height;
        } else {
            arguments[index].lowerBound = index * chunkSize;
            arguments[index].upperBound = (index + 1) * chunkSize;
        }

        arguments[index].workerId = index;
        arguments[index].runner = runner;
        pthread_create(&runner->workers[index], 0, simulate_chunk, &arguments[index]);
    }

    while (runner->currentTurn < runner->turnsCount) {
        //Waiting for workers to start a new iteration
        for (uint index = 0; index < runner->workersCount; ++index) {
            sem_wait(runner->startSem + index);
        }

        ++runner->currentTurn;

        //Wait for all workers to finish
        for (uint index = 0; index < runner->workersCount; ++index) {
            sem_wait(runner->middleSem + index);
        }

        swap_fields(runner);

        //Resume workers;
        for (uint index = 0; index < runner->workersCount; ++index) {
            sem_post(runner->finishSem + index);
        }
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
