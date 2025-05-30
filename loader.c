#pragma once

#include <flecs.h>

#include "components.c"
#include "helpers.c"

void register_resources(ecs_world_t *world) {
	ecs_singleton_set(world, GameState, {
		.screen = Playing,
		.position = (Position) { .x = DISPLAY_CENTER_X, .y = DISPLAY_CENTER_Y },
		.fullscren = false
	});
}
