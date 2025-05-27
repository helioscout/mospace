#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <time.h>

#include <raylib.h>
#include <box2d/box2d.h>
#include <flecs.h>

#include "types.h"
#include "components.c"
#include "queries.c"
#include "sprites.c"
#include "helpers.c"

float time_step = 1.0f / 60.0f;
int sub_step_count = 4;
int bullets_interval = 100;			// Interval between bullet shots in milliseconds.
clock_t shot_time = 0;				// Last shot time.

b2WorldId world_id;
ecs_world_t* world;

void init_player() {
	sprite_t *sprite = sprite_get("ship-a");
	ecs_entity_t player = ecs_entity(world, { .name = "player" });

	ecs_set(world, player, Sprite,		{ .texture = sprite->texture });
	ecs_set(world, player, Size,		{ .width = sprite->width, .height = sprite->height });
	ecs_set(world, player, Position,	{ .x = DISPLAY_CENTER_X - sprite->width / 2, .y = DISPLAY_CENTER_Y - sprite->height / 2 });
	ecs_set(world, player, Center,		{ .cx = sprite->width / 2, .cy = sprite->height / 2 });
	ecs_set(world, player, Rotation,	{ .angle = 0.0f });
	ecs_set(world, player, Weapon,		{ .type = w_one_bullet });
	ecs_set(world, player, Ship,		{ .speed = 50, .tracing = false, .trace = { .tint = 0 } });
	
	Ship *ship = ecs_get_mut(world, player, Ship);
	
	for (int i = 0; i < 10; i++) {
		ship->trace.texture[i] = sprites.trace_thin[i];
		ship->trace.width[i] = ship->trace.texture[i].width;
		ship->trace.height[i] = ship->trace.texture[i].height;
	}
}

void init_physics() {
	ecs_entity_t player = ecs_lookup(world, "player");
	
	b2WorldDef world_def = b2DefaultWorldDef();
	world_def.gravity = (b2Vec2){ 0.0f, 0.0f };

	world_id = b2CreateWorld(&world_def);

	b2BodyDef body_def = b2DefaultBodyDef();
	body_def.userData = user_data(player);
	body_def.type = b2_dynamicBody;
	body_def.position = (b2Vec2){ pixels_to_meters(DISPLAY_CENTER_X), pixels_to_meters(DISPLAY_CENTER_Y) };

	b2BodyId body_id = b2CreateBody(world_id, &body_def);

	ecs_set(world, player, Physics, { .body_id = body_id });

	const Size *size = ecs_get(world, player, Size);

	b2Polygon dynamic_box = b2MakeBox(pixels_to_meters(size->width) / 2, pixels_to_meters(size->height) / 2);
	b2ShapeDef shape_def = b2DefaultShapeDef();
	shape_def.density = 1.0f;
	shape_def.material.friction = 0.1f;

	b2CreatePolygonShape(body_id, &shape_def, &dynamic_box);
}

void destroy_physics() {
	b2DestroyWorld(world_id);
	world_id = b2_nullWorldId;
}

void init_asteroid(ecs_entity_t asteroid) {
	const Position *pos = ecs_get(world, asteroid, Position);
	const Size *size = ecs_get(world, asteroid, Size);

	b2BodyDef body_def = b2DefaultBodyDef();
	body_def.userData = user_data(asteroid);
	body_def.type = b2_dynamicBody;
	body_def.position = (b2Vec2){ pixels_to_meters(pos->x + size->width / 2), pixels_to_meters(pos->y + size->height / 2) };

	b2BodyId body_id = b2CreateBody(world_id, &body_def);

	ecs_set(world, asteroid, Physics, { .body_id = body_id });

	b2Polygon dynamic_box = b2MakeBox(pixels_to_meters(size->width) / 2, pixels_to_meters(size->height) / 2);
	b2ShapeDef shape_def = b2DefaultShapeDef();
	shape_def.density = 1.0f;
	shape_def.material.friction = 0.01f;
	shape_def.enableContactEvents = true;

	b2CreatePolygonShape(body_id, &shape_def, &dynamic_box);

	b2MassData mass_data;
	mass_data.mass = 100.0f;
	mass_data.center = (b2Vec2){ 0.0f, 0.0f };
	mass_data.rotationalInertia = 50.0f;

	b2Body_SetMassData(body_id, mass_data);
}

void create_asteroids() {
	for (int i = 0; i < 4; i ++) {
		int x, y;

		switch (i) {
			case 0: x = 150; y = 150; break;
			case 1: x = 600; y = 150; break;
			case 2: x = 200; y = 400; break;
			case 3: x = 550; y = 400; break;
		}
		
		sprite_t *sprite = sprite_get(i % 2 == 1 ? "meteor-detailed-large" : "meteor-detailed-small");
		ecs_entity_t asteroid = ecs_new(world);

		ecs_add_id(world, asteroid, Asteroid);
		ecs_set(world, asteroid, Position,	{ .x = x, .y = y });
		ecs_set(world, asteroid, Size,		{ .width = sprite->width, .height = sprite->height });
		ecs_set(world, asteroid, Center,	{ .cx = sprite->width / 2, .cy = sprite->height / 2 });
		ecs_set(world, asteroid, Rotation,	{ .angle = 0.0f });
		ecs_set(world, asteroid, Sprite,	{ .texture = sprite->texture });

		init_asteroid(asteroid);
	}
}

void create_spark(int x, int y) {
	ecs_entity_t spark = ecs_new(world);

	ecs_add_id(world, spark, Spark);
	ecs_set(world, spark, Position,		{ .x = x, .y = y });
	ecs_set(world, spark, Animation,	{ .frame = 0, .speed = 2, .count = 3, .textures = &sprites.spark });
}

void init_bullet(ecs_entity_t bullet) {
	const Position *pos = ecs_get(world, bullet, Position);
	const Size *size = ecs_get(world, bullet, Size);
	const Rotation *rot = ecs_get(world, bullet, Rotation);
	
	b2BodyDef body_def = b2DefaultBodyDef();
	body_def.userData = user_data(bullet);
	body_def.type = b2_dynamicBody;
	body_def.position = (b2Vec2){ pixels_to_meters(pos->x + size->width / 2), pixels_to_meters(pos->y + size->height / 2) };
	body_def.rotation = b2MakeRot(rot->angle);
	body_def.isBullet = true;

	b2BodyId body_id = b2CreateBody(world_id, &body_def);

	ecs_set(world, bullet, Physics, { .body_id = body_id });

	b2Polygon dynamic_box = b2MakeBox(pixels_to_meters(size->width) / 2, pixels_to_meters(size->height) / 2);
	b2ShapeDef shape_def = b2DefaultShapeDef();
	shape_def.density = 1.0f;
	shape_def.material.friction = 0.01f;
	shape_def.filter.groupIndex = -1;
	shape_def.enableContactEvents = true;

	b2CreatePolygonShape(body_id, &shape_def, &dynamic_box);
	
	b2Body_ApplyLinearImpulseToCenter(body_id, b2RotateVector(b2MakeRot(degrees_to_radians(-90)), angle_to_vector(rot->angle, 20.0)), true);
}

bool bullet_shot_allowed() {
	if (!shot_time) return true;

	double duration = 1000.0 * (clock() - shot_time) / CLOCKS_PER_SEC;
	return duration >= bullets_interval;
}

void player_shot() {
	sprite_t *sprite = sprite_get("bullet-a");
	ecs_entity_t player = ecs_lookup(world, "player");
	
	const Position *position = ecs_get(world, player, Position);
	const Center *center = ecs_get(world, player, Center);
	const Rotation *rot = ecs_get(world, player, Rotation);
	const Size *size = ecs_get(world, player, Size);
	const Weapon *weapon = ecs_get(world, player, Weapon);
	
	if (weapon->type == w_one_bullet) {
		if (!bullet_shot_allowed()) return;
		
		Point pos = {
			.x = position->x + center->cx - sprite->width / 2,
			.y = position->y - sprite->height
		};

		rotate_point_in(&pos, position->x + center->cx, position->y + center->cy, rot->angle);

		ecs_entity_t bullet = ecs_new(world);

		ecs_add_id(world, bullet, Bullet);
		ecs_set(world, bullet, Position,	{ .x = pos.x, .y = pos.y });
		ecs_set(world, bullet, Size,		{ .width = sprite->width, .height = sprite->height });
		ecs_set(world, bullet, Center,		{ .cx = sprite->width / 2, .cy = sprite->height / 2 });
		ecs_set(world, bullet, Rotation,	{ .angle = rot->angle });
		ecs_set(world, bullet, Sprite,		{ .texture = sprite->texture });

		init_bullet(bullet);
		
		shot_time = clock();
	} else if (weapon->type == w_two_bullets) {
		if (!bullet_shot_allowed()) return;

		int dx = size->width / 3;

		Point pos1 = {
			.x = position->x + dx - sprite->width / 2,
			.y = position->y - sprite->height
		};

		Point pos2 = {
			.x = position->x + 2 * dx - sprite->width / 2,
			.y = position->y - sprite->height
		};

		rotate_point_in(&pos1, position->x + center->cx, position->y + center->cy, rot->angle);
		rotate_point_in(&pos2, position->x + center->cx, position->y + center->cy, rot->angle);
		
		ecs_entity_t bullet1 = ecs_new(world);

		ecs_add_id(world, bullet1, Bullet);
		ecs_set(world, bullet1, Position,	{ .x = pos1.x, .y = pos1.y });
		ecs_set(world, bullet1, Size,		{ .width = sprite->width, .height = sprite->height });
		ecs_set(world, bullet1, Center,		{ .cx = sprite->width / 2, .cy = sprite->height / 2 });
		ecs_set(world, bullet1, Rotation,	{ .angle = rot->angle });
		ecs_set(world, bullet1, Sprite,		{ .texture = sprite->texture });
		
		ecs_entity_t bullet2 = ecs_new(world);

		ecs_add_id(world, bullet2, Bullet);
		ecs_set(world, bullet2, Position,	{ .x = pos2.x, .y = pos2.y });
		ecs_set(world, bullet2, Size,		{ .width = sprite->width, .height = sprite->height });
		ecs_set(world, bullet2, Center,		{ .cx = sprite->width / 2, .cy = sprite->height / 2 });
		ecs_set(world, bullet2, Rotation,	{ .angle = rot->angle });
		ecs_set(world, bullet2, Sprite,		{ .texture = sprite->texture });

		init_bullet(bullet1);
		init_bullet(bullet2);
		
		shot_time = clock();
	}
}

void process_player() {
	ecs_entity_t player = ecs_lookup(world, "player");

	b2BodyId body_id = ecs_get(world, player, Physics)->body_id;
	// Defined experimentally based on ship_a (it's movement) for which it equals 30.
	float impulse = 3.90625 * b2Body_GetMass(body_id);
	float angular_impulse = 0.6510417 * b2Body_GetMass(body_id);

	Weapon *weapon = ecs_get_mut(world, player, Weapon);
	Ship *ship = ecs_get_mut(world, player, Ship);
	
	if (IsKeyPressed(KEY_ONE)) weapon->type = w_one_bullet;
	else if (IsKeyPressed(KEY_TWO)) weapon->type = w_two_bullets;
	
	if (IsKeyDown(KEY_A)) b2Body_ApplyLinearImpulseToCenter(body_id, b2RotateVector(b2Body_GetRotation(body_id), (b2Vec2){ -impulse, 0.0f }), true);
	if (IsKeyDown(KEY_D)) b2Body_ApplyLinearImpulseToCenter(body_id, b2RotateVector(b2Body_GetRotation(body_id), (b2Vec2){ impulse, 0.0f }), true);
	if (IsKeyDown(KEY_UP)) b2Body_ApplyLinearImpulseToCenter(body_id, b2RotateVector(b2Body_GetRotation(body_id), (b2Vec2){ 0.0f, -impulse }), true);
	if (IsKeyDown(KEY_DOWN)) b2Body_ApplyLinearImpulseToCenter(body_id, b2RotateVector(b2Body_GetRotation(body_id), (b2Vec2){ 0.0f, impulse }), true);
	if (IsKeyDown(KEY_RIGHT)) b2Body_ApplyAngularImpulse(body_id, angular_impulse, true);
	if (IsKeyDown(KEY_LEFT)) b2Body_ApplyAngularImpulse(body_id, -angular_impulse, true);

	if (IsKeyPressed(KEY_MINUS)) ship->speed = 0;
	else if (IsKeyPressed(KEY_EQUAL)) ship->speed = 50;

	if (IsKeyDown(KEY_Q)) {
		if (ship->speed > 0) ship->speed--;
	} else if (IsKeyDown(KEY_E)) {
		if (ship->speed < 50) ship->speed++;
	}

	// Emergency braking.
	if (IsKeyDown(KEY_SPACE)) {
		float linear_damping = b2Body_GetLinearDamping(body_id);
		float angular_damping = b2Body_GetAngularDamping(body_id);
		
		if (linear_damping < 100) b2Body_SetLinearDamping(body_id, linear_damping * 1.2f + 0.5f);
		if (angular_damping < 100) b2Body_SetAngularDamping(body_id, angular_damping * 1.2f + 0.5f);
	} else {
		b2Body_SetLinearDamping(body_id, (50 - ship->speed) / 10.0f );
		b2Body_SetAngularDamping(body_id, (50 - ship->speed) / 10.0f);
	}

	if (IsKeyDown(KEY_W)) player_shot();
	
	ship->tracing = IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_A) || IsKeyDown(KEY_D);
}

void clean_entities() {
	ecs_query_t *q = ecs_query(world, {
		.terms = {
			{ .id = ecs_id(Bullet), .inout = EcsInOutNone },
			{ .id = ecs_id(Position), .inout = EcsIn },
			{ .id = ecs_id(Size), .inout = EcsIn },
			{ .id = ecs_id(Physics), .inout = EcsIn }
		},
		.cache_kind = EcsQueryCacheAuto
	});

	ecs_iter_t iter = ecs_query_iter(world, q);

	while (ecs_query_next(&iter)) {
		const Position *pos = ecs_field(&iter, Position, 1);
		const Size *size = ecs_field(&iter, Size, 2);
		const Physics *physics = ecs_field(&iter, Physics, 3);

		for (int i = 0; i < iter.count; i++) {
			if (query_is_outside_of_rect(&pos[i], &size[i], &(Rectangle){ .x = 0, .y = 0, .width = DISPLAY_WIDTH, .height = DISPLAY_HEIGHT })) {
				free_user_data(physics[i].body_id);
				b2DestroyBody(physics[i].body_id);
				ecs_delete(world, iter.entities[i]);
			}
		}
	}

	ecs_query_fini(q);
}

void process_physics() {
	b2World_Step(world_id, time_step, sub_step_count);
	
	ecs_entity_t player = ecs_lookup(world, "player");
	
	Position *pos = ecs_get_mut(world, player, Position);
	Rotation *rot = ecs_get_mut(world, player, Rotation);
	const Center *center = ecs_get(world, player, Center);
	const Physics *physics = ecs_get(world, player, Physics);

	b2Vec2 position = b2Body_GetPosition(physics->body_id);
	b2Rot rotation = b2Body_GetRotation(physics->body_id);

	pos->x = meters_to_pixels(position.x) - center->cx;
	pos->y = meters_to_pixels(position.y) - center->cy;
	rot->angle = b2Rot_GetAngle(rotation);

	ecs_query_t *q_b = ecs_query(world, {
		.terms = {
			{ .id = ecs_id(Bullet), .inout = EcsInOutNone },
			{ .id = ecs_id(Position), .inout = EcsOut },
			{ .id = ecs_id(Rotation), .inout = EcsOut },
			{ .id = ecs_id(Center), .inout = EcsIn },
			{ .id = ecs_id(Physics), .inout = EcsIn }
		},
		.cache_kind = EcsQueryCacheAuto
	});

	ecs_iter_t iter_b = ecs_query_iter(world, q_b);

	while (ecs_query_next(&iter_b)) {
		Position *pos = ecs_field(&iter_b, Position, 1);
		Rotation *rot = ecs_field(&iter_b, Rotation, 2);
		const Center *center = ecs_field(&iter_b, Center, 3);
		const Physics *physics = ecs_field(&iter_b, Physics, 4);

		for (int i = 0; i < iter_b.count; i++) {
			b2Vec2 position = b2Body_GetPosition(physics[i].body_id);
			b2Rot rotation = b2Body_GetRotation(physics[i].body_id);

			pos[i].x = meters_to_pixels(position.x) - center[i].cx;
			pos[i].y = meters_to_pixels(position.y) - center[i].cy;
			rot[i].angle = b2Rot_GetAngle(rotation);
		}
	}

	ecs_query_fini(q_b);

	ecs_query_t *q_a = ecs_query(world, {
		.terms = {
			{ .id = ecs_id(Asteroid), .inout = EcsInOutNone },
			{ .id = ecs_id(Position), .inout = EcsOut },
			{ .id = ecs_id(Rotation), .inout = EcsOut },
			{ .id = ecs_id(Center), .inout = EcsIn },
			{ .id = ecs_id(Physics), .inout = EcsIn }
		},
		.cache_kind = EcsQueryCacheAuto
	});

	ecs_iter_t iter_a = ecs_query_iter(world, q_a);

	while (ecs_query_next(&iter_a)) {
		Position *pos = ecs_field(&iter_a, Position, 1);
		Rotation *rot = ecs_field(&iter_a, Rotation, 2);
		const Center *center = ecs_field(&iter_a, Center, 3);
		const Physics *physics = ecs_field(&iter_a, Physics, 4);

		for (int i = 0; i < iter_a.count; i++) {
			b2Vec2 position = b2Body_GetPosition(physics[i].body_id);
			b2Rot rotation = b2Body_GetRotation(physics[i].body_id);

			pos[i].x = meters_to_pixels(position.x) - center[i].cx;
			pos[i].y = meters_to_pixels(position.y) - center[i].cy;
			rot[i].angle = b2Rot_GetAngle(rotation);
		}
	}

	ecs_query_fini(q_a);
}

bool check_bullet_asteroid_collision(ecs_entity_t *entity_a, ecs_entity_t *entity_b) {
	ecs_entity_t bullet = 0;

	if (ecs_is_valid(world, *entity_a) && ecs_is_valid(world, *entity_b)) {
		if (ecs_has_id(world, *entity_a, Bullet) && ecs_has_id(world, *entity_b, Asteroid)) bullet = *entity_a;
		else if (ecs_has_id(world, *entity_b, Bullet) && ecs_has_id(world, *entity_a, Asteroid)) bullet = *entity_b;
	}

	if (bullet == 0) return false;

	const Position *pos = ecs_get(world, bullet, Position);
	const Center *center = ecs_get(world, bullet, Center);
	const Physics *physics = ecs_get(world, bullet, Physics);

	create_spark(pos->x + center->cx, pos->y + center->cy);

	free_user_data(physics->body_id);
	b2DestroyBody(physics->body_id);
	ecs_delete(world, bullet);

	return true;
}

void process_collisions() {
	b2ContactEvents events = b2World_GetContactEvents(world_id);

	for (int i = 0; i < events.beginCount; i++) {
		b2ContactBeginTouchEvent *event = events.beginEvents + i;
		b2BodyId body_id_a = b2Shape_GetBody(event->shapeIdA);
		b2BodyId body_id_b = b2Shape_GetBody(event->shapeIdB);

		ecs_entity_t *entity_a = b2Body_GetUserData(body_id_a);
		ecs_entity_t *entity_b = b2Body_GetUserData(body_id_b);

		check_bullet_asteroid_collision(entity_a, entity_b);
	}
}

void process_animations() {
	ecs_query_t *q = ecs_query(world, {
		.terms = {
			{ .id = ecs_id(Spark), .inout = EcsInOutNone },
			{ .id = ecs_id(Animation), .inout = EcsOut }
		},
		.cache_kind = EcsQueryCacheAuto
	});
	
	ecs_iter_t iter = ecs_query_iter(world, q);

	while (ecs_query_next(&iter)) {
		Animation *a = ecs_field(&iter, Animation, 1);

		for (int i = 0; i < iter.count; i++) {
			a[i].frame++;

			if (a[i].frame == a[i].count * a[i].speed)
				ecs_delete(world, iter.entities[i]);
		}
	}

	ecs_query_fini(q);
}

void draw_player() {
	ecs_entity_t player = ecs_lookup(world, "player");
	
	const Rotation *rot = ecs_get(world, player, Rotation);
	const Sprite *sprite = ecs_get(world, player, Sprite);
	const Position *pos = ecs_get(world, player, Position);
	const Center *center = ecs_get(world, player, Center);
	const Size *size = ecs_get(world, player, Size);
	Ship *ship = ecs_get_mut(world, player, Ship);

	if (rot->angle == 0.0f) DrawTexture(sprite->texture, pos->x, pos->y, WHITE);
	else DrawTexturePro(
		sprite->texture,
		(Rectangle) { .x = 0.0f, .y = 0.0f, .width = sprite->texture.width, .height = sprite->texture.height },
		(Rectangle) { .x = pos->x + center->cx, .y = pos->y + center->cy, .width = sprite->texture.width, .height = sprite->texture.height },
		(Vector2) { .x = center->cx, .y = center->cy },
		radians_to_degrees(rot->angle),
		WHITE);

	if (ship->tracing) {
		int sprite_index = (ship->speed - 1) / 5;
		float width = ship->trace.width[sprite_index];
		float height = ship->trace.height[sprite_index];
		Pointf position = rotate_pointf(
			pos->x + center->cx - width / 2.0f,
			pos->y + size->height,
			pos->x + center->cx,
			pos->y + center->cy,
			rot->angle);
		
		if (ship->trace.tint < 255) ship->trace.tint += 5;
		
		DrawTexturePro(
			ship->trace.texture[sprite_index],
			(Rectangle) { .x = 0.0f, .y = 0.0f, .width = width, .height = height },
			(Rectangle) { .x = position.x, .y = position.y, .width = width, .height = height },
			(Vector2) { .x = 0.0f, .y = 0.0f },
			radians_to_degrees(rot->angle),
			(Color) { .r = 255 - ship->trace.tint, .g = 255, .b = 255, .a = 255 });
	} else ship->trace.tint = 0;
}

void draw_bullets() {
	ecs_query_t *q = ecs_query(world, {
		.terms = {
			{ .id = ecs_id(Bullet), .inout = EcsInOutNone },
			{ .id = ecs_id(Position), .inout = EcsIn },
			{ .id = ecs_id(Rotation), .inout = EcsIn },
			{ .id = ecs_id(Center), .inout = EcsIn },
			{ .id = ecs_id(Sprite), .inout = EcsIn }
		},
		.cache_kind = EcsQueryCacheAuto
	});

	ecs_iter_t iter = ecs_query_iter(world, q);

	while (ecs_query_next(&iter)) {
		const Position *pos = ecs_field(&iter, Position, 1);
		const Rotation *rot = ecs_field(&iter, Rotation, 2);
		const Center *center = ecs_field(&iter, Center, 3);
		const Sprite *sprite = ecs_field(&iter, Sprite, 4);

		for (int i = 0; i < iter.count; i++) {
			if (rot[i].angle == 0.0f) DrawTexture(sprite[i].texture, pos[i].x, pos[i].y, WHITE);
			else DrawTexturePro(
				sprite[i].texture,
				(Rectangle) { .x = 0.0f, .y = 0.0f, .width = sprite[i].texture.width, .height = sprite[i].texture.height },
				(Rectangle) { .x = pos[i].x + center[i].cx, .y = pos[i].y + center[i].cy, .width = sprite[i].texture.width, .height = sprite[i].texture.height },
				(Vector2) { .x = center[i].cx, .y = center[i].cy },
				radians_to_degrees(rot[i].angle),
				WHITE);
		}
	}

	ecs_query_fini(q);
}

void draw_asteroids() {
	ecs_query_t *q = ecs_query(world, {
		.terms = {
			{ .id = ecs_id(Asteroid), .inout = EcsInOutNone },
			{ .id = ecs_id(Position), .inout = EcsIn },
			{ .id = ecs_id(Rotation), .inout = EcsIn },
			{ .id = ecs_id(Center), .inout = EcsIn },
			{ .id = ecs_id(Sprite), .inout = EcsIn }
		},
		.cache_kind = EcsQueryCacheAuto
	});

	ecs_iter_t iter = ecs_query_iter(world, q);

	while (ecs_query_next(&iter)) {
		const Position *pos = ecs_field(&iter, Position, 1);
		const Rotation *rot = ecs_field(&iter, Rotation, 2);
		const Center *center = ecs_field(&iter, Center, 3);
		const Sprite *sprite = ecs_field(&iter, Sprite, 4);

		for (int i = 0; i < iter.count; i++) {
			if (rot[i].angle == 0.0f) DrawTexture(sprite[i].texture, pos[i].x, pos[i].y, WHITE);
			else DrawTexturePro(
				sprite[i].texture,
				(Rectangle) { .x = 0.0f, .y = 0.0f, .width = sprite[i].texture.width, .height = sprite[i].texture.height },
				(Rectangle) { .x = pos[i].x + center[i].cx, .y = pos[i].y + center[i].cy, .width = sprite[i].texture.width, .height = sprite[i].texture.height },
				(Vector2) { .x = center[i].cx, .y = center[i].cy },
				radians_to_degrees(rot[i].angle),
				WHITE);
		}
	}

	ecs_query_fini(q);
}

void draw_sparks() {
	ecs_query_t *q = ecs_query(world, {
		.terms = {
			{ .id = ecs_id(Spark), .inout = EcsInOutNone },
			{ .id = ecs_id(Position), .inout = EcsIn },
			{ .id = ecs_id(Animation), .inout = EcsIn }
		},
		.cache_kind = EcsQueryCacheAuto
	});

	ecs_iter_t iter = ecs_query_iter(world, q);

	while (ecs_query_next(&iter)) {
		const Position *pos = ecs_field(&iter, Position, 1);
		const Animation *a = ecs_field(&iter, Animation, 2);

		for (int i = 0; i < iter.count; i++) {
			Texture2D texture = (*a[i].textures)[a[i].frame / a[i].speed];
			DrawTexture(texture, pos[i].x - texture.width / 2.0, pos[i].y - texture.height / 2.0, WHITE);
		}
	}

	ecs_query_fini(q);
}

int main() {
	world = ecs_init();
	
	register_components(world);
	
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(DISPLAY_WIDTH, DISPLAY_HEIGHT, "mospace");
	SetTargetFPS(60);
	
	sprites_init();
	init_player();
	init_physics();

	create_asteroids();

	while (!WindowShouldClose()) {
		clean_entities();
		process_animations();
		process_player();
		process_physics();
		process_collisions();

		BeginDrawing();
		ClearBackground(BLACK);

		draw_player();
		draw_bullets();
		draw_asteroids();
		draw_sparks();

		EndDrawing();
	}

	sprites_destroy();
	destroy_physics();
	ecs_fini(world);

	CloseWindow();

	return EXIT_SUCCESS;
}
