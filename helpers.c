#pragma once

#include <math.h>

#include <box2d/box2d.h>
#include <flecs.h>

#include "types.h"

const int DISPLAY_WIDTH = 800;
const int DISPLAY_HEIGHT = 600;
int DISPLAY_CENTER_X = DISPLAY_WIDTH / 2;
int DISPLAY_CENTER_Y = DISPLAY_HEIGHT / 2;
float SCALING_FACTOR = 0.1f;

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
