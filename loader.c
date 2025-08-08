#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <flecs.h>
#include <raylib.h>
#include <sqlite3.h>

#include "components.c"
#include "sprites.c"
#include "helpers.c"
#include "layout.c"
#include "db.c"

static int sql_sprites([[maybe_unused]] void *data, int count, char **fields, char **cols){
	char *key;
	char *label;
	char *file_name;
	
	for (size_t i = 0; i < (size_t)count; i++) {
		if (strcmp(cols[i], "key") == 0) key = fields[i];
		else if (strcmp(cols[i], "label") == 0) label = fields[i];
		else if (strcmp(cols[i], "file_name") == 0) file_name = fields[i];
	}

	sprite_new(key, label, file_name);

	return 0;
}

void register_resources(ecs_world_t *world, Camera2D *camera) {
	sqlite3 *db;
	char* err;

	sprites_init();

	if (sqlite3_open("space.db", &db) != SQLITE_OK) {
		printf("Couldn't open database: %s.\n", sqlite3_errmsg(db));
		exit(EXIT_FAILURE);
	}

	if (sqlite3_exec(db, "select key, label, file_name from sprite", sql_sprites, NULL, &err) != SQLITE_OK) {
		printf("SQLite error: %s.\n", err);
		sqlite3_free(err);
		exit(EXIT_FAILURE);
	}

	sqlite3_close(db);

	window_t *w_menu = malloc(sizeof(window_t));
	gui_menu_t *gui_menu = malloc(sizeof(gui_menu_t));
	world_t *map = malloc(sizeof(world_t));
	
	*w_menu = (window_t) {
		.position	= { .x = 10, .y = 10 },
		.size		= { .x = 290, .y = 140 },
		.minimized	= false,
		.moving		= false,
		.resizing	= false
	};
	
	*gui_menu = (gui_menu_t) {
		.worlds_editing = false,
		.world_index    = 0,
		.world_ids      = {},
		.world_labels   = {},
		.worlds_count   = 0
	};

	*map = (world_t) {
		.id		  = 0,
		.label	  = NULL,
		.width	  = 0,
		.height	  = 0
	};

	ecs_singleton_set(world, GameState, {
	    .camera    = camera,
		.screen    = Menu,
		.actions   = Nothing,
		.position  = (Vector2) { .x = DISPLAY_CENTER_X, .y = DISPLAY_CENTER_Y },
		.size	   = (Vector2) { .x = DISPLAY_WIDTH, .y = DISPLAY_HEIGHT },
		.fullscren = false,
		.zoom      = 1.0f,
		.scaled    = 0,
		.loaded	   = false,
		.map	   = map
	});

	GameGui *gui = malloc(sizeof(GameGui));
	*gui = (GameGui) {
		.w_menu	  = w_menu,
		.gui_menu = gui_menu,
		.layout   = gui_layout
	};

	ecs_set_id(world, ecs_id(GameGui), ecs_id(GameGui), sizeof(GameGui), gui);

	load_worlds(gui);
}
