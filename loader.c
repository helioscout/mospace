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

static int sql_sprites(void *data, int count, char **fields, char **cols){
	char *key;
	char *label;
	char *file_name;
	
	for (int i = 0; i < count; i++) {
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
	
	ecs_singleton_set(world, GameState, {
	    .camera = camera,
		.screen = Playing,
		.position = (Vector2) { .x = DISPLAY_CENTER_X, .y = DISPLAY_CENTER_Y },
		.fullscren = false,
		.zoom = 1.0f,
		.scaled = 0
	});
}
