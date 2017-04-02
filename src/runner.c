#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <ncurses.h>
#include "runner.h"
#include "field.h"
#include "life.h"


Runner* create_empty_runner(uint width, uint height, uint turnsCount, uint workersCount, uint mult) {
    Runner *result = (Runner*)malloc(sizeof(Runner));

    result->turnsCount = turnsCount;
    result->currentTurn = 0;

    result->mult = mult;
    
    result->field = create_empty_field(width, height);
    result->tmpField = create_empty_field(width, height);

    result->workersCount = workersCount;
    result->workers = (pthread_t*)malloc(sizeof(pthread_t) * workersCount);

    pthread_barrier_init(&result->start, NULL, workersCount + 1);
    pthread_barrier_init(&result->finish, NULL, workersCount + 1);

    return result;
}

Runner* create_random_runner(uint width, uint height, uint turnsCount, uint workersCount,uint mult) {
    Runner *result = create_empty_runner(width, height, turnsCount, workersCount, mult);

    result->field = create_random_configuration(width, height);

    return result;
}

Runner* create_glider_test_runner(uint width, uint height, uint turnsCount, uint workersCount, uint mult) {
    Runner *result = create_empty_runner(width, height, turnsCount, workersCount, mult);

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

    pthread_barrier_destroy(&runner->start);
    pthread_barrier_destroy(&runner->finish);

    free(runner);
}

typedef struct _simulation_arguments_struct {
    uint lowerBound;
    uint upperBound;
    Runner *runner;
} SimulationsArguments;

void* simulate_chunk(void *arguments) {
    SimulationsArguments *args = (SimulationsArguments*)arguments;
    while (args->runner->currentTurn < args->runner->turnsCount) {
        pthread_barrier_wait(&args->runner->start);

        simulate_step(args->runner->field, args->runner->tmpField, args->lowerBound, args->upperBound);

        pthread_barrier_wait(&args->runner->finish);
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
    //print_state(runner);
    while (runner->currentTurn < runner->turnsCount) {
        //Resume all workers
        pthread_barrier_wait(&runner->start);
        
        ++runner->currentTurn;

        //Wait for all workers to finish
        pthread_barrier_wait(&runner->finish);
        
        //print if -x opt
        if( runner->mult != 0) {
	    clear();
            print_state_nc(runner);
            refresh(); 
            struct timespec t;
            struct timespec* t2;
            t.tv_sec = 0;
            assert(runner->mult < 20);
            t.tv_nsec = 100000000L / runner->mult ;
            nanosleep(&t,t2);
        }
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
    
    printf("\n");
}

void print_state_nc(const Runner *runner) {
    printw("Turn %d:\n", runner->currentTurn);

    for (uint yPos = 0; yPos < runner->field->height; ++yPos) {
        for (uint xPos = 0; xPos < runner->field->width; ++xPos) {
            if (get_cell(runner->field, xPos, yPos) == CS_ALIVE) {
                printw("X");
            } else {
                printw(".");
            }
        }
        printw("\n");
    }
    
    printw("\n");
}
