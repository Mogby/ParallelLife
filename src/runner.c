#include <stdlib.h>
#include <stdio.h>
#include "runner.h"
#include "field.h"
#include "life.h"

Runner* create_runner(uint width, uint height) {
    Runner *result = (Runner*)malloc(sizeof(Runner));

    result->turnsCount = 0;
    result->field = create_random_configuration(width, height);
    result->tmpField = create_empty_field(width, height);

    return result;
}

void destroyRunner(Runner *runner) {
    destroy_field(runner->field);
    destroy_field(runner->tmpField);
    free(runner);
}

void proceed_one_step(Runner *runner) {
    simulate_step(runner->field, runner->tmpField, 0, runner->field->width * runner->field->height);

    Field *tmp = runner->field;
    runner->field = runner->tmpField;
    runner->tmpField = tmp;

    ++runner->turnsCount;
}

void print_state(const Runner *runner) {
    printf("Turn %d:\n", runner->turnsCount);

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
