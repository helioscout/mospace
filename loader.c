#pragma once

#include <flecs.h>
#include <raylib.h>

#include "components.c"
#include "helpers.c"

void register_resources(ecs_world_t *world, Camera2D *camera) {
	ecs_singleton_set(world, GameState, {
	    .camera = camera,
		.screen = Playing,
		.position = (Vector2) { .x = DISPLAY_CENTER_X, .y = DISPLAY_CENTER_Y },
		.fullscren = false,
		.zoom = 1.0f,
		.scaled = 0
	});
}
