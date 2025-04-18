#include <math.h>

#include <box2d/box2d.h>
#include <flecs.h>

#include "types.h"

const int display_width = 800;
const int display_height = 600;
int display_center_x = display_width / 2;
int display_center_y = display_height / 2;
float scaling_factor = 0.1f;

float pixels_to_meters(int pixels) { return pixels * scaling_factor; }
int meters_to_pixels(float meters) { return (int)(meters / scaling_factor); }

Point rotate_point(int x, int y, int cx, int cy, float angle) {
	return (Point) {
		.x = lroundf(cosf(angle) * (x - cx) - sinf(angle) * (y - cy) + cx),
		.y = lroundf(sinf(angle) * (x - cx) + cosf(angle) * (y - cy) + cy)
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

ecs_entity_t* user_data(ecs_entity_t entity) {
	ecs_entity_t *data = malloc(sizeof(ecs_entity_t));
	*data = entity;

	return data;
}

void free_user_data(b2BodyId body_id) {
	ecs_entity_t *entity = b2Body_GetUserData(body_id);
	free(entity);
}
