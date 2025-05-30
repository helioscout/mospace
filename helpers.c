#pragma once

#include <math.h>
#include <time.h>

#include <box2d/box2d.h>
#include <flecs.h>

#include "components.c"
#include "types.h"

const int DISPLAY_WIDTH = 800;
const int DISPLAY_HEIGHT = 600;
const int DISPLAY_CENTER_X = DISPLAY_WIDTH / 2;
const int DISPLAY_CENTER_Y = DISPLAY_HEIGHT / 2;
const float SCALING_FACTOR = 0.1f;
const float TIME_STEP = 1.0f / 60.0f;
const int SUB_STEP_COUNT = 4;
const int SHOT_INTERVAL = 100;			// Interval between shots in milliseconds.
const int ZOOM_INTERVAL = 100;			// Interval between camera zoom in milliseconds.

float pixels_to_meters(int pixels) { return pixels * SCALING_FACTOR; }
int meters_to_pixels(float meters) { return (int)(meters / SCALING_FACTOR); }

Point rotate_point(int x, int y, int cx, int cy, float angle) {
	return (Point) {
		.x = lroundf(cosf(angle) * (x - cx) - sinf(angle) * (y - cy) + cx),
		.y = lroundf(sinf(angle) * (x - cx) + cosf(angle) * (y - cy) + cy)
	};
}

Pointf rotate_pointf(float x, float y, float cx, float cy, float angle) {
	return (Pointf) {
		.x = cosf(angle) * (x - cx) - sinf(angle) * (y - cy) + cx,
		.y = sinf(angle) * (x - cx) + cosf(angle) * (y - cy) + cy
	};
}

void rotate_point_in(Point *point, int cx, int cy, float angle) {
	int	x = point->x,
		y = point->y;
	
	point->x = lroundf(cosf(angle) * (x - cx) - sinf(angle) * (y - cy) + cx);
	point->y = lroundf(sinf(angle) * (x - cx) + cosf(angle) * (y - cy) + cy);
}

b2Vec2 angle_to_vector(float angle, float scale) {
	return (b2Vec2) {
		.x = cosf(angle) * scale,
		.y = sinf(angle) * scale
	};
}

float degrees_to_radians(int degrees) {
	return degrees * M_PI / 180;
}

float radians_to_degrees(float radians) {
	return radians * 180 / M_PI;
}

ecs_entity_t* user_data(ecs_entity_t entity) {
	ecs_entity_t *data = malloc(sizeof(ecs_entity_t));
	*data = entity;

	return data;
}

void free_user_data(b2BodyId body_id) {
	ecs_entity_t *entity = b2Body_GetUserData(body_id);
	free(entity);
}

bool shot_allowed(clock_t shot_time) {
	if (!shot_time) return true;

	double duration = 1000.0 * (clock() - shot_time) / CLOCKS_PER_SEC;
	return duration >= SHOT_INTERVAL;
}

bool zoom_allowed(clock_t zoom_time) {
	if (!zoom_time) return true;

	double duration = 1000.0 * (clock() - zoom_time) / CLOCKS_PER_SEC;
	return duration >= ZOOM_INTERVAL;
}

bool is_outside_of_rect(const Position *pos, const Size *size, const Rectangle *rect) {
	return	pos->x + size->width < rect->x  ||
			pos->x > rect->x + rect->width  ||
			pos->y + size->height < rect->y ||
			pos->y > rect->y + rect->height;
}
