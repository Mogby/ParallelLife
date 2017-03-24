#ifndef RUNNER_H

#define RUNNER_H

typedef unsigned int uint;

typedef struct _field_struct Field;

typedef struct _runner_struct {
    uint turnsCount;
    Field *field;
    Field *tmpField;
} Runner;

Runner* create_runner(uint width, uint height);

void destroy_runner(Runner *runner);

void proceed_one_step(Runner *runner);

void print_state(const Runner *runner);

#endif
