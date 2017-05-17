#include <stdlib.h>
#include "field.h"
#include "shm_malloc.h"

Field *create_empty_field(uint width, uint height, char initialize) {
    ECellStatus **cells = (ECellStatus**)shm_malloc(sizeof(ECellStatus*) * width);

    for (uint xPos = 0; xPos < width; ++xPos) {
        cells[xPos] = (ECellStatus*)shm_malloc(sizeof(ECellStatus) * height);
        if (initialize) {
            for (uint yPos = 0; yPos < height; ++yPos) {
                cells[xPos][yPos] = CS_DEAD;
            }
        }
    }

    Field* result = (Field*)shm_malloc(sizeof(Field));
    if (initialize) {
        result->width = width;
        result->height = height;
        result->cells = cells;
    }

    return result;
}

char on_field(const Field *field, int xPos, int yPos) {
    return 0 <= xPos && xPos < field->width &&
           0 <= yPos && yPos < field->height ? 1 : 0;
}

ECellStatus get_cell(const Field *field, int xPos, int yPos) {
    if (!on_field(field, xPos, yPos)) {
        return CS_OUT_OF_RANGE;
    }

    return field->cells[xPos][yPos];
}

void set_cell(const Field *field, int xPos, int yPos, char alive) {
    if (!on_field(field, xPos, yPos)) {
        return;
    }

    field->cells[xPos][yPos] = alive ? CS_ALIVE : CS_DEAD;
}
