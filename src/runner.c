#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "shm_malloc.h"

#include "runner.h"
#include "field.h"
#include "life.h"

Runner *create_empty_runner(uint width, uint height, uint turnsCount, uint workersCount, Parameters *parameters,
                            char initialize) {
    Runner *result = (Runner*)shm_malloc(sizeof(Runner));

    if (initialize) {
        result->turnsCount = turnsCount;
        result->currentTurn = 0;
    }

    result->field = create_empty_field(width, height, initialize);
    result->tmpField = create_empty_field(width, height, initialize);

    if (initialize) {
        result->workersCount = workersCount;
    }

    result->workers = (pid_t*)shm_malloc(sizeof(pid_t) * workersCount);

    result->startSem = (sem_t**)shm_malloc(sizeof(sem_t*) * workersCount);
    result->middleSem = (sem_t**)shm_malloc(sizeof(sem_t*) * workersCount);
    result->finishSem = (sem_t**)shm_malloc(sizeof(sem_t*) * workersCount);

    if (initialize) {
        for (uint index = 0; index < workersCount; ++index) {
            result->startSem[index] = shm_create_semaphore();
            result->middleSem[index] = shm_create_semaphore();
            result->finishSem[index] = shm_create_semaphore();
        }
    }

    result->parameters = parameters;

    return result;
}

Runner *create_random_runner(uint width, uint height, uint turnsCount, uint workersCount, Parameters *parameters,
                             char initialize) {
    Runner *result = create_empty_runner(width, height, turnsCount, workersCount, parameters, initialize);

    if (initialize) {
        result->field = create_random_configuration(width, height, initialize);
    }

    return result;
}

Runner *create_glider_test_runner(uint width, uint height, uint turnsCount, uint workersCount, Parameters *parameters,
                                  char initialize) {
    Runner *result = create_empty_runner(width, height, turnsCount, workersCount, parameters, initialize);

    if (initialize) {
        set_cell(result->field, 1, 0, 1);
        set_cell(result->field, 2, 1, 1);
        set_cell(result->field, 2, 2, 1);
        set_cell(result->field, 1, 2, 1);
        set_cell(result->field, 0, 2, 1);
    }

    return result;
}

typedef struct _simulation_arguments_struct {
    uint lowerBound;
    uint upperBound;
    uint workerId;
    Runner *runner;
} SimulationArguments;

void* simulate_chunk(SimulationArguments *args) {
   while (args->runner->currentTurn < args->runner->turnsCount) {
        sem_post(args->runner->startSem[args->workerId]);

        simulate_step(args->runner->field, args->runner->tmpField, args->lowerBound, args->upperBound);

        sem_post(args->runner->middleSem[args->workerId]);

        sem_wait(args->runner->finishSem[args->workerId]);
    }

    return NULL;
}

void swap_fields(Runner *runner) {
    Field *temporary = runner->field;
    runner->field = runner->tmpField;
    runner->tmpField = temporary;
}

int run(Runner *runner) {
    SimulationArguments *arguments = malloc(sizeof(SimulationArguments) * runner->workersCount);
    uint chunkSize = runner->field->width * runner->field->height / runner->workersCount;

    assert(runner->workersCount);

    pid_t pid;
    for (uint index = 0; index < runner->workersCount; ++index) {
        if (index == runner->workersCount - 1) {
            //If area of field is not divisible by number of workers, the last worker must take all remaining cells
            arguments[index].lowerBound = index * chunkSize;
            arguments[index].upperBound = runner->field->width * runner->field->height;
        } else {
            arguments[index].lowerBound = index * chunkSize;
            arguments[index].upperBound = (index + 1) * chunkSize;
        }

        arguments[index].workerId = index;
        arguments[index].runner = runner;

        pid = fork();
        if (pid) {
            runner->workers[index] = pid;
        } else {
            simulate_chunk(arguments + index);
            return 0;
        }
    }

    while (runner->currentTurn < runner->turnsCount) {
        //Waiting for workers to start a new iteration
        for (uint index = 0; index < runner->workersCount; ++index) {
            sem_wait(runner->startSem[index]);
        }

        ++runner->currentTurn;

        //Wait for all workers to finish
        for (uint index = 0; index < runner->workersCount; ++index) {
            sem_wait(runner->middleSem[index]);
        }

        swap_fields(runner);

        //Resume workers;
        for (uint index = 0; index < runner->workersCount; ++index) {
            sem_post(runner->finishSem[index]);
        }
    }

    for (uint index = 0; index < runner->workersCount; ++index) {
        waitpid(runner->workers[index], NULL, 0);
    }

    return 1;
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
