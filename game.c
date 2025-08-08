#include <stdlib.h>
#include <stddef.h>

#include <raylib.h>
#include <flecs.h>

#include "components.c"
#include "systems.c"
#include "loader.c"
#include "helpers.c"
#include "gui.c"

int main() {
	// ecs_log_set_level(0);

	ecs_world_t* world = ecs_init();
	
	Camera2D camera = (Camera2D) {
		.zoom = 1.0f,
	};

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	SetTraceLogLevel(LOG_WARNING | LOG_ERROR | LOG_FATAL);
	InitWindow(DISPLAY_WIDTH, DISPLAY_HEIGHT, "mospace");
	SetExitKey(KEY_NULL);
	SetTargetFPS(60);
	GuiLoadStyleCyber();

	register_components(world);
	register_resources(world, &camera);
	systems_t sys = register_systems(world);

	/* Enable for physics debug drawings. */
	ecs_enable(world, sys.debug, false);

	while (!WindowShouldClose() && !SHOULD_EXIT_GAME) {
		BeginDrawing();
		ClearBackground(BLACK);
		BeginMode2D(camera);
		
		ecs_progress(world, 0.0f);

		EndMode2D();
		EndDrawing();
	}

	ecs_run(world, sys.destroy, 0.0f, NULL);
	ecs_fini(world);

	// ecs_log_set_level(-1);

	CloseWindow();

	return EXIT_SUCCESS;
}
