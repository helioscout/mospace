#pragma once

#include <flecs.h>

#include "components.c"

void register_resources(ecs_world_t *world) {
	ecs_singleton_set(world, GameState, {
		.screen = Playing
	});
}
