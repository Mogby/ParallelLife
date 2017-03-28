#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "runner.h"
#include "field.h"
#include "life.h"

Runner* create_runner(uint width, uint height, uint turnsCount, uint workersCount) {
    Runner *result = (Runner*)malloc(sizeof(Runner));

    result->turnsCount = turnsCount;
    result->currentTurn = 0;

    result->field = create_random_configuration(width, height);
    result->tmpField = create_empty_field(width, height);

    result->workersCount = workersCount;
    result->workers = (pthread_t*)malloc(sizeof(pthread_t) * workersCount);

    pthread_barrier_init(&result->barrier, NULL, workersCount + 1);

    sem_init(&result->semaphore, 0, 0);

    return result;
}

void destroy_runner(Runner *runner) {
    destroy_field(runner->field);
    destroy_field(runner->tmpField);

    free(runner->workers);

    pthread_barrier_destroy(&runner->barrier);
    sem_destroy(&runner->semaphore);

    free(runner);
}

void proceed_one_step(Runner *runner, int lowerBound, int upperBound) {
    simulate_step(runner->field, runner->tmpField, lowerBound, upperBound);
}

typedef struct _simulation_arguments_struct {
    int lowerBound;
    int upperBound;
    Runner *runner;
} SimulationsArguments;

void* simulate_chunk(void *arguments) {
    SimulationsArguments *args = (SimulationsArguments*)arguments;
    while (args->runner->currentTurn < args->runner->turnsCount) {
        sem_wait(&args->runner->semaphore);

        simulate_step(args->runner->field, args->runner->tmpField, args->lowerBound, args->upperBound);

        pthread_barrier_wait(&args->runner->barrier);
    }

    return NULL;
}

void swap_fields(Runner *runner) {
    Field *temporary = runner->field;
    runner->field = runner->tmpField;
    runner->tmpField = temporary;
}

void run(Runner *runner) {
    SimulationsArguments *arguments = malloc(sizeof(SimulationsArguments) * runner->workersCount);
    uint chunkSize = runner->field->width * runner->field->height / runner->workersCount;

    assert(runner->workersCount);


    for (uint index = 0; index < runner->workersCount - 1; ++index) {
        arguments[index].lowerBound = index * chunkSize;
        arguments[index].upperBound = (index + 1) * chunkSize;
        arguments[index].runner = runner;
        pthread_create(&runner->workers[index], 0, simulate_chunk, &arguments[index]);
    }

    //If area of field is not divisible by number of workers, the last worker must take all remaining cells
    arguments[runner->workersCount - 1].lowerBound = (runner->workersCount - 1) * chunkSize;
    arguments[runner->workersCount - 1].upperBound = runner->field->width * runner->field->height;
    arguments[runner->workersCount - 1].runner = runner;

    pthread_create(&runner->workers[runner->workersCount - 1], 0,
                   simulate_chunk, &arguments[runner->workersCount - 1]);

    //Printing initial state
    print_state(runner);
    while (runner->currentTurn < runner->turnsCount) {
        //Resume all workers
        for (uint count = 0; count < runner->workersCount; ++count) {
            sem_post(&runner->semaphore);
        }
        
        ++runner->currentTurn;

        //Wait for all workers to finish
        pthread_barrier_wait(&runner->barrier);

        swap_fields(runner);

        print_state(runner);
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
