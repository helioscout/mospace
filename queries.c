#pragma once

#include <stdbool.h>

#include "types.h"
#include "components.c"

bool query_is_outside_of_rect(const Position *pos, const Size *size, const Rectangle *rect) {
	return	pos->x + size->width < rect->x  ||
			pos->x > rect->x + rect->width  ||
			pos->y + size->height < rect->y ||
			pos->y > rect->y + rect->height;
}
