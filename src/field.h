#ifndef FIELD_H

#define FIELD_H

typedef unsigned int uint;

typedef enum _cell_status_enum {
    CS_DEAD,
    CS_ALIVE,
    CS_OUT_OF_RANGE
} ECellStatus;

typedef struct _field_struct {
    uint width;
    uint height;

    ECellStatus **cells;
} Field;

Field *create_empty_field(uint width, uint height, char initialize);

void destroy_field(Field *field);

ECellStatus get_cell(const Field *field, int xPos, int yPos);

void set_cell(const Field *field, int xPos, int yPos, char alive);

#endif
