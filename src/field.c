#include <stdlib.h>
#include "field.h"

Field* create_empty_field(uint width, uint height) {
    ECellStatus **cells = (ECellStatus**)malloc(sizeof(ECellStatus*) * width);

    for (uint xPos = 0; xPos < width; ++xPos) {
        cells[xPos] = (ECellStatus*)malloc(sizeof(ECellStatus) * height);
        for (uint yPos = 0; yPos < height; ++yPos) {
            cells[xPos][yPos] = CS_DEAD;
        }
    }

    Field* result = (Field*)malloc(sizeof(Field));
    result->width = width;
    result->height = height;
    result->cells = cells;

    return result;
}

void destroy_field(Field *field) {
    for (uint xPos = 0; xPos < field->width; ++xPos) {
        free(field->cells[xPos]);
    }
    free(field->cells);
    free(field);
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
