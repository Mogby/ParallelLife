# Parallel Life

Conway's game of life implementation that runs in several threads.

## Building

The code uses pthread and argp, so it can only be compiled on systems where they are available.

Run CMake and then build the program with your favourite build system.

## Running

Usage: life [OPTION...]

Options:  
* _-t, --threads-count_ = **THREADS** &mdash; Number of threads (default: 1)
* _-w, --field-width_ = **WIDTH** &mdash; Width of game field (default: 10)
* _-h, --field-height_ = **HEIGHT** &mdash; Height of game field (default: 10)
* _-l, --game-length_ = **TURNS** &mdash; Number of turns to simulate (default: 20)
* _--glider-test_ &mdash; If specified, the program will run glider test
* _-?, --help_ &mdash; Give this help list
* _--usage_ &mdash; Give a short usage message

If the _--glider-test_ option is specified, the program will simulate a glider on
a field of the given size, that flies from the top left corner in bottom right
direction for the given number of turns.

Otherwise, the initial configuration of cells is chosen randomly.
