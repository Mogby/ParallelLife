#include <stdio.h>
#include "runner.h"

int main(int argc, char *argv) {
    Runner *runner = create_runner(9, 9, 10, 1);
    run(runner);
    destroy_runner(runner);

    return 0;
}

