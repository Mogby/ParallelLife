#include <stdlib.h>
#include <argp.h>
#include "runner.h"
#include "shm_malloc.h"

error_t parse_argument(int key, char *arg, struct argp_state *state) {
    Parameters *parameters = (Parameters*)state->input;

    int value;
    switch(key) {
        case 'p':
            value = atoi(arg);
            if (value <= 0) {
                return EINVAL;
            }
            parameters->threadsCount = value;
            break;
        case 'w':
            value = atoi(arg);
            if (value <= 0) {
                return EINVAL;
            }
            parameters->fieldWidth = value;
            break;
        case 'h':
            value = atoi(arg);
            if (value <= 0) {
                return EINVAL;
            }
            parameters->fieldHeight = value;
            break;
        case 'l':
            value = atoi(arg);
            if (value <= 0) {
                return EINVAL;
            }
            parameters->turnsCount = value;
            break;
        case -1:
            parameters->gliderTest = 1;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

Parameters get_parameters(int argc, char **argv) {
    Parameters parameters = { 1, 10, 10, 20, 0 };

    struct argp_option options[] = {
            { "processes-count", 'p', "PROCESSES", 0, "Number of processes (default: 1)", 0 },
            { "field-width", 'w', "WIDTH", 0, "Width of game field (default: 10)", 1 },
            { "field-height", 'h', "HEIGHT", 0, "Height of game field (default: 10)", 2 },
            { "game-length", 'l', "TURNS", 0, "Number of turns to simulate (default: 20)", 3 },
            { "glider-test", -1, 0, OPTION_ARG_OPTIONAL, "If specified, the program will run glider test", 4 },
            { 0 }
    };

    char doc[] =
            "A program that simulates John Conway's Game of Life"
                    "\vIf the --glider-test option is specified, the program will "
                    "simulate a glider on a field of the given size, that flies from "
                    "the top left corner in bottom right direction for the given "
                    "number of turns.\n\n"
                    "Otherwise, the initial configuration of cells is chosen randomly.";

    struct argp parser = { options, parse_argument, NULL, doc };

    argp_parse(&parser, argc, argv, 0, NULL, (void*)&parameters);

    return parameters;
}

int main(int argc, char **argv) {
    Parameters newParameters;
    newParameters.gliderTest = 0;
    newParameters = get_parameters(argc, argv);
    newParameters.isCorrect = 0;

    Parameters *oldParameters = shm_try_open(sizeof(newParameters));
    Parameters *parameters;

    char initialize = 1;
    if (oldParameters &&
        oldParameters->isCorrect &&
        oldParameters->fieldHeight == newParameters.fieldHeight &&
        oldParameters->fieldWidth == newParameters.fieldWidth &&
        oldParameters->turnsCount == newParameters.turnsCount &&
        oldParameters->gliderTest == newParameters.gliderTest &&
        oldParameters->threadsCount == newParameters.threadsCount) {
        parameters = oldParameters;
        initialize = 0;
    } else {
        parameters = shm_malloc(sizeof(Parameters));
        *parameters = newParameters;
    }

    if (parameters->gliderTest && parameters->fieldWidth < 3 || parameters->fieldHeight < 3) {
        //Glider won't fit on a field of the given size.
        return EINVAL;
    }

    Runner *runner;
    if (parameters->gliderTest) {
        runner = create_glider_test_runner(parameters->fieldWidth, parameters->fieldHeight, parameters->turnsCount,
                                           parameters->threadsCount, parameters, initialize);
    } else {
        runner = create_random_runner(parameters->fieldWidth, parameters->fieldHeight, parameters->turnsCount,
                                      parameters->threadsCount, parameters, initialize);
    }

    if (!run(runner)) {
        shm_close_sems();
        shm_free_sems();
        //shm_unlink_all();
        shm_free_records();

        return 0;
    }

    print_state(runner);

    shm_close_sems();
    shm_destroy_sems();
    shm_free_sems();

    //shm_unmap_and_close_all();
    //shm_unlink_all();
    shm_free_records();

    return 0;
}
