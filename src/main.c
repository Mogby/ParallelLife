#include <stdio.h>
#include "runner.h"

int main() {
    Runner *runner = create_runner(3, 3);
    for (uint count = 0; count < 10; ++count) {
        print_state(runner);
        proceed_one_step(runner);
    }
    destroy_runner(runner);
    return 0;
}

