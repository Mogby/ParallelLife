#include <stdlib.h>
#include <argp.h>
#include <ncurses.h>
#include "runner.h"

typedef struct _parameters_struct {
    uint threadsCount;
    uint fieldWidth;
    uint fieldHeight;
    uint turnsCount;
    uint mult;
    char gliderTest;
} Parameters;

error_t parse_argument(int key, char *arg, struct argp_state *state) {
    Parameters *parameters = (Parameters*)state->input;

    int value;
    switch(key) {
        case 't':
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
        case 'x':
            value = atoi(arg);
            if (value <= 0) {
                return EINVAL;
            }
            parameters->mult = value;
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
    Parameters parameters = { 1, 10, 10, 20, 0,0 };

    struct argp_option options[] = {
            { "threads-count", 't', "THREADS", 0, "Number of threads (default: 1)", 0 },
            { "field-width", 'w', "WIDTH", 0, "Width of game field (default: 10)", 1 },
            { "field-height", 'h', "HEIGHT", 0, "Height of game field (default: 10)", 2 },
            { "game-length", 'l', "TURNS", 0, "Number of turns to simulate (default: 20)", 3 },
            { "glider-test", -1, 0, OPTION_ARG_OPTIONAL, "If specified, the program will run glider test", 4 },
            { "multiple-number", 'x', "MULT", 0, "Mutiple number to speed up output", 5 },
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
    Parameters parameters = get_parameters(argc, argv);

    if (parameters.gliderTest && parameters.fieldWidth < 3 || parameters.fieldHeight < 3) {
        //Glider won't fit on a field of the given size.
        return EINVAL;
    }

    Runner *runner;
    if (parameters.gliderTest) {
        runner = create_glider_test_runner(parameters.fieldWidth, parameters.fieldHeight,
                                           parameters.turnsCount, parameters.threadsCount, parameters.mult);
    } else {
        runner = create_random_runner(parameters.fieldWidth, parameters.fieldHeight,
                                      parameters.turnsCount, parameters.threadsCount, parameters.mult);
    }
    if(parameters.mult != 0){
        initscr();
    }
    run(runner);
    if(parameters.mult != 0){
        endwin();
    } else {
        print_state(runner);
    }
    destroy_runner(runner);
    return 0;
}
