#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#include <raylib.h>
#include <flecs.h>

#include "components.c"
#include "systems.c"
#include "loader.c"

int main() {
	ecs_world_t* world = ecs_init();
	
	Camera2D camera = (Camera2D) {
		.zoom = 1.0f,
	};

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	SetTraceLogLevel(LOG_WARNING | LOG_ERROR | LOG_FATAL);
	InitWindow(DISPLAY_WIDTH, DISPLAY_HEIGHT, "mospace");
	SetTargetFPS(60);

	register_components(world);
	register_resources(world, &camera);
	systems_t sys = register_systems(world);
	
	ecs_enable(world, sys.debug, false);

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		BeginMode2D(camera);
		
		ecs_progress(world, 0.0f);

		EndMode2D();
		EndDrawing();
	}

	ecs_run(world, sys.destroy, 0.0f, NULL);
	ecs_fini(world);

	CloseWindow();

	return EXIT_SUCCESS;
}
