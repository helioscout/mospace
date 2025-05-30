#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#include <raylib.h>
#include <flecs.h>

#include "components.c"
#include "systems.c"
#include "sprites.c"
#include "loader.c"

int main() {
	ecs_world_t* world = ecs_init();
	
	register_components(world);
	register_resources(world);
	systems_t sys = register_systems(world);
	
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(DISPLAY_WIDTH, DISPLAY_HEIGHT, "mospace");
	SetTargetFPS(60);

	sprites_init();

	ecs_enable(world, sys.debug, false);

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		
		ecs_progress(world, 0.0f);

		EndDrawing();
	}

	ecs_run(world, sys.destroy, 0.0f, NULL);
	ecs_fini(world);

	CloseWindow();

	return EXIT_SUCCESS;
}
