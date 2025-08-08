#pragma once

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <sqlite3.h>
#include <flecs.h>

#include "components.c"
#include "sprites.c"
#include "helpers.c"

typedef struct sql_param_t {
	ecs_world_t* world;
	b2WorldId world_id;
} sql_param_t;

static int sql_worlds(void *data, int count, char **fields, char **cols) {
	gui_menu_t *gui_menu = (gui_menu_t*)data;
	
	for (size_t i = 0; i < (size_t)count; i++) {
		if (strcmp(cols[i], "id") == 0) {
			sscanf(fields[i], "%llu", &gui_menu->world_ids[gui_menu->worlds_count]);
		} else if (strcmp(cols[i], "label") == 0) {
			char *label = malloc(strlen(fields[i]) + 1);

			label[0] = '\0';
			strcpy(label, fields[i]);
			
			gui_menu->world_labels[gui_menu->worlds_count] = label;
			gui_menu->worlds_count++;
		}
	}

	return 0;
}

void load_worlds(GameGui *gui) {
	sqlite3 *db;
	char* err;

	/* We need to clear existing worlds list. */
	for (size_t i = 0; i < gui->gui_menu->worlds_count; ++i) {
		free(gui->gui_menu->world_labels[i]);
	}

	gui->gui_menu->worlds_count = 0;
	
	if (sqlite3_open("space.db", &db) != SQLITE_OK) {
		printf("Couldn't open database: %s.\n", sqlite3_errmsg(db));
		exit(EXIT_FAILURE);
	}

	if (sqlite3_exec(db, "select id, label from world", sql_worlds, gui->gui_menu, &err) != SQLITE_OK) {
		printf("SQLite error: %s.\n", err);
		sqlite3_free(err);
		exit(EXIT_FAILURE);
	}

	sqlite3_close(db);
	
    // After reloading drop down list items we need to point to the first item.
    gui->gui_menu->world_index = 0;
}

static int sql_world(void *data, int count, char **fields, char **cols) {
	world_t *map = (world_t*)data;

	for (size_t i = 0; i < (size_t)count; i++) {
		if (strcmp(cols[i], "label") == 0) {
			if (map->label) free(map->label);
			map->label = malloc(strlen(fields[i]) + 1);

			map->label[0] = '\0';
			strcpy(map->label, fields[i]);
		} else if (strcmp(cols[i], "width") == 0) {
			sscanf(fields[i], "%f", &map->width);
		} else if (strcmp(cols[i], "height") == 0) {
			sscanf(fields[i], "%f", &map->height);
		}
	}

	return 0;
}

static int sql_entities(void *data, int count, char **fields, char **cols) {
	sql_param_t *param = (sql_param_t*)data;
	ecs_world_t *world = param->world;
	b2WorldId world_id = param->world_id;
	int x = 0, y = 0;
	double angle = 0.0;
	char *key;
	bool player = false;
	bool asteroid = false;
	bool enemy = false;

	ecs_entity_t entity = ecs_new(world);

	for (size_t i = 0; i < (size_t)count; i++) {
		if (strcmp(cols[i], "x") == 0) sscanf(fields[i], "%d", &x);
		else if (strcmp(cols[i], "y") == 0) sscanf(fields[i], "%d", &y);
		else if (strcmp(cols[i], "angle") == 0) sscanf(fields[i], "%lf", &angle);
		else if (strcmp(cols[i], "sprite_key") == 0) key = fields[i];
		/* Reading tags, - all next columns except tag.id. */
		else if (strcmp(cols[i], "id") != 0) {
			int value;
			sscanf(fields[i], "%d", &value);

			if (strcmp(cols[i], "player") == 0) {
				if (value) {
					player = true;
					ecs_add_id(world, entity, Player);
				}
			} else if (strcmp(cols[i], "asteroid") == 0) {
				if (value) {
					asteroid = true;
					ecs_add_id(world, entity, Asteroid);
				}
			} else if (strcmp(cols[i], "enemy") == 0) {
				if (value) {
					enemy = true;
					ecs_add_id(world, entity, Enemy);
				}
			}
		}
	}

	sprite_t *sprite = sprite_get(key);
	
	int cx = sprite->width / 2;
	int cy = sprite->height / 2;
	int width = sprite->width;
	int height = sprite->height;

	/* Adding components specific to all entities. */
	ecs_set(world, entity, Position,	{ .x = x, .y = y });
	ecs_set(world, entity, Size,		{ .width = width, .height = height });
	ecs_set(world, entity, Center,		{ .cx = cx, .cy = cy });
	ecs_set(world, entity, Rotation,	{ .angle = angle });
	ecs_set(world, entity, Sprite,		{ .texture = sprite->texture });

	/* Ship with trace and weapon are specific only for player and enemy. */
	if (player || enemy) {
		Trace trace = (Trace) { .tint = 0 };

		for (size_t i = 0; i < 10; i++) {
			trace.texture[i] = sprites.trace_thin[i];
			trace.width[i] = trace.texture[i].width;
			trace.height[i] = trace.texture[i].height;
		}

		ecs_set(world, entity, Weapon,	{ .kind = OneBullet, .shot = 0 });
		ecs_set(world, entity, Ship,	{ .speed = 50, .tracing = false, .trace = trace });
	}

	/* Actions is specific only for player. */
	if (player) {
		ecs_set(world, entity, Actions,	{ .actions = Nothing });
	}

	/* Creating entity physics body. */
	b2BodyDef body_def = b2DefaultBodyDef();
	body_def.userData = user_data(entity);
	body_def.type = b2_dynamicBody;
	body_def.position = (b2Vec2) {
		.x = pixels_to_meters(x + cx),
		.y = pixels_to_meters(y + cy) };
	body_def.rotation = b2MakeRot(angle);

	b2BodyId body_id = b2CreateBody(world_id, &body_def);

	/* Adding physics handle component to link entity with it's physics. */
	ecs_set(world, entity, Handle, { .body_id = body_id });

	/* Creating body collider. */
	b2Polygon dynamic_box = b2MakeBox(pixels_to_meters(width) / 2, pixels_to_meters(height) / 2);
	b2ShapeDef shape_def = b2DefaultShapeDef();
	shape_def.density = 1.0f;
	shape_def.material.friction = 0.1f;
	shape_def.enableContactEvents = true;

	b2CreatePolygonShape(body_id, &shape_def, &dynamic_box);

	/* Setting mass for asteroid depending on it's size. */
	if (asteroid) {
		b2MassData mass_data;
		mass_data.mass = width * height / 9.8f;
		mass_data.center = (b2Vec2) { 0.0f, 0.0f };
		mass_data.rotationalInertia = 50.0f;

		b2Body_SetMassData(body_id, mass_data);
	}
	
	return 0;
}

/** Loads the world from the database.
 * @param state	   ~ pointer to game state.
 * @param world    ~ pointer to entity of the flecs world.
 * @param world_id ~ box2d world id to add new bodies.
 * @param id       ~ selected in drop down world id to use in sql. */
void load_world(GameState *state, ecs_world_t *world, b2WorldId world_id, uint64_t id) {
	sqlite3 *db;
	char* err;
	char* sql = malloc(1000);
	char* num = malloc(100);

	if (sqlite3_open("space.db", &db) != SQLITE_OK) {
		printf("Couldn't open database: %s.\n", sqlite3_errmsg(db));
		exit(EXIT_FAILURE);
	}

	strcpy(sql, "select label, width, height from world where id = ");
	sprintf(num, "%llu", id);
	strcat(sql, num);

	if (sqlite3_exec(db, sql, sql_world, state->map, &err) != SQLITE_OK) {
		printf("SQLite error: %s.\n", err);
		sqlite3_free(err);
		exit(EXIT_FAILURE);
	}

	state->map->id = id;

	strcpy(sql,
		"select p.x as x, p.y as y, r.angle as angle, sprite_key, t.* "
		"from entity e "
		"inner join position p on p.id = e.position_id "
		"inner join rotation r on r.id = e.rotation_id "
		"inner join tag t on t.id = e.tag_id "
		"where e.world_id = ");
	strcat(sql, num);

	if (sqlite3_exec(db, sql, sql_entities, &(sql_param_t) { .world = world, .world_id = world_id }, &err) != SQLITE_OK) {
		printf("SQLite error: %s.\n", err);
		sqlite3_free(err);
		exit(EXIT_FAILURE);
	}

	sqlite3_close(db);
	free(sql);
	free(num);
}
