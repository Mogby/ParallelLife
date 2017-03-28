# Parallel Life

Conway's game of life implementation that runs in several threads.

## Building

The code uses pthread and arpg, so it can only be compiled on systems where they are available.

Run CMake and then build the program with your favourite build system.

## Running

Usage: life [OPTION...]

Options:  
* -t, --threads-count=THREADS   Number of threads (default: 1)
* -w, --field-width=WIDTH    Width of game field (default: 10)
* -h, --field-height=HEIGHT  Height of game field (default: 10)
* -l, --game-length=TURNS    Number of turns to simulate (default: 20)
* --glider-test          If specified, the program will run glider test
* -?, --help                 Give this help list
* --usage                Give a short usage message

If the --glider-test option is specified, the program will simulate a glider on
a field of the given size, that flies from the top left corner in bottom right
direction for the given number of turns.

Otherwise, the initial configuration of cells is chosen randomly.
