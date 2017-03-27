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

    pthread_barrier_t barr;

    return result;
}

void destroy_runner(Runner *runner) {
    destroy_field(runner->field);
    destroy_field(runner->tmpField);
    free(runner->workers);
    free(runner);
}

void proceed_one_step(Runner *runner) {
    simulate_step(runner->field, runner->tmpField, 0, runner->field->width * runner->field->height);

    Field *tmp = runner->field;
    runner->field = runner->tmpField;
    runner->tmpField = tmp;

    ++runner->currentTurn;
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
