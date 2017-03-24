#ifndef LIFE_H

#define LIFE_H

typedef unsigned int uint;

typedef struct _field_struct Field;

Field* create_random_configuration(uint width, uint height);

uint neighbours_count(const Field *field, int xPos, int yPos);

void simulate_step(const Field *src_field, Field *dst_field, uint lowerBound, uint upperBound);

#endif
