#pragma once

#include <stdbool.h>

#include "types.h"
#include "memory.c"

bool query_is_outside_of_rect(Entity *entity, void *param) {
	Rectangle *rect = param;
	Position *pos = component_get(entity, c_position);
	Size *size = component_get(entity, c_size);

	return	pos->x + size->width < rect->x  ||
			pos->x > rect->x + rect->width  ||
			pos->y + size->height < rect->y ||
			pos->y > rect->y + rect->height;
}
