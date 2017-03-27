#include <stdlib.h>
#include <assert.h>
#include "life.h"
#include "field.h"

Field* create_random_configuration(uint width, uint height) {
    Field *field = create_empty_field(width, height);

    for (int xPos = 0; xPos < width; ++xPos) {
        for (int yPos = 0; yPos < height; ++yPos) {
            if (rand() & 1) {
                set_cell(field, xPos, yPos, 1);
            }
        }
    }

    return field;
}

uint neighbours_count(const Field *field, int xPos, int yPos) {
    uint result = 0;

    for (int deltaX = -1; deltaX <= 1; ++deltaX) {
        for (int deltaY = -1; deltaY <= 1; ++deltaY) {
            if ((deltaX || deltaY) && get_cell(field, xPos + deltaX, yPos + deltaY) == CS_ALIVE) {
                ++result;
            }
        }
    }

    return result;
}

void simulate_step(const Field *src_field, Field *dst_field, uint lowerBound, uint upperBound) {
    assert(src_field->width == dst_field->width && src_field->height == dst_field->height);
    assert(src_field->width && src_field->height);

    uint xPos = lowerBound / src_field->height;
    uint yPos = lowerBound % src_field->height;
    uint neighboursCount;

    while (lowerBound < upperBound) {
        neighboursCount = neighbours_count(src_field, xPos, yPos);

        assert(get_cell(src_field, xPos, yPos) != CS_OUT_OF_RANGE);
        if (get_cell(src_field, xPos, yPos) == CS_ALIVE) {
            if (neighboursCount == 2 || neighboursCount == 3) {
                set_cell(dst_field, xPos, yPos, 1);
            } else {
                set_cell(dst_field, xPos, yPos, 0);
            }
        } else {
            if (neighboursCount == 3) {
                set_cell(dst_field, xPos, yPos, 1);
            } else {
                set_cell(dst_field, xPos, yPos, 0);
            }
        }

        ++lowerBound;
        ++yPos;
        if (yPos == src_field->height) {
            yPos = 0;
            ++xPos;
        }
    }
}
