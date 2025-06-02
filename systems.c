#pragma once

#include <time.h>
#include <stdio.h>
#include <math.h>

#include <raylib.h>
#include <flecs.h>
#include <box2d/box2d.h>

#include "components.c"
#include "sprites.c"
#include "helpers.c"
#include "debug.c"

typedef struct systems_t {
	ecs_entity_t load;
	ecs_entity_t create;
	ecs_entity_t generate;
	ecs_entity_t control;
	ecs_entity_t actions;
	ecs_entity_t physics;
	ecs_entity_t transformation;
	ecs_entity_t camera;
	ecs_entity_t draw;
	ecs_entity_t debug;
	ecs_entity_t shooting;
	ecs_entity_t collisions;
	ecs_entity_t effects;
	ecs_entity_t cleaning;
	ecs_entity_t destroy;
} systems_t;

void load(ecs_iter_t *iter) {
	ecs_world_t *world = iter->world;

	sprite_t *sprite = sprite_get("player-ship-a-bw");
	ecs_entity_t player = ecs_new(world);

	Trace trace = (Trace) { .tint = 0 };

	for (int i = 0; i < 10; i++) {
		trace.texture[i] = sprites.trace_thin[i];
		trace.width[i] = trace.texture[i].width;
		trace.height[i] = trace.texture[i].height;
	}

	ecs_add_id(world, player, Player);
	ecs_set(world, player, Sprite,		{ .texture = sprite->texture });
	ecs_set(world, player, Size,		{ .width = sprite->width, .height = sprite->height });
	ecs_set(world, player, Position,	{ .x = DISPLAY_CENTER_X - sprite->width / 2, .y = DISPLAY_CENTER_Y - sprite->height / 2 });
	ecs_set(world, player, Center,		{ .cx = sprite->width / 2, .cy = sprite->height / 2 });
	ecs_set(world, player, Rotation,	{ .angle = 0.0f });
	ecs_set(world, player, Weapon,		{ .kind = OneBullet, .shot = 0 });
	ecs_set(world, player, Ship,		{ .speed = 50, .tracing = false, .trace = trace });
	ecs_set(world, player, Actions,		{ .actions = Nothing });
	ecs_set(world, player, Handle,		{ .body_id = b2_nullBodyId });
	
	for (int i = 0; i < 4; i ++) {
		int x, y;

		switch (i) {
			case 0: x = 150; y = 150; break;
			case 1: x = 600; y = 150; break;
			case 2: x = 200; y = 400; break;
			case 3: x = 550; y = 400; break;
		}
		
		sprite_t *sprite = sprite_get(i % 2 == 1 ? "asteroid-detailed-large-bw" : "asteroid-detailed-small-bw");
		ecs_entity_t asteroid = ecs_new(world);

		ecs_add_id(world, asteroid, Asteroid);
		ecs_set(world, asteroid, Position,	{ .x = x, .y = y });
		ecs_set(world, asteroid, Size,		{ .width = sprite->width, .height = sprite->height });
		ecs_set(world, asteroid, Center,	{ .cx = sprite->width / 2, .cy = sprite->height / 2 });
		ecs_set(world, asteroid, Rotation,	{ .angle = 0.0f });
		ecs_set(world, asteroid, Sprite,	{ .texture = sprite->texture });
		ecs_set(world, asteroid, Handle,	{ .body_id = b2_nullBodyId });
	}

	// printf("load\n");
}

void create(ecs_iter_t *iter) {
	ecs_world_t *world = iter->world;
	
	b2WorldDef world_def = b2DefaultWorldDef();
	world_def.gravity = (b2Vec2) { 0.0f, 0.0f };

	b2WorldId world_id = b2CreateWorld(&world_def);

	b2DebugDraw	debug_drawer = b2DefaultDebugDraw();
	debug_drawer.drawShapes = true;
	debug_drawer.DrawPolygonFcn = draw_polygon;
	debug_drawer.DrawSolidPolygonFcn = draw_solid_polygon;
	debug_drawer.DrawSegmentFcn = draw_segment;
	debug_drawer.DrawPointFcn = draw_point;

	ecs_singleton_set(world, Space, {
		.world_id = world_id,
		.debug_drawer = debug_drawer
	});

	// printf("create\n");
}

void generate(ecs_iter_t *iter) {
	const Position *pos = ecs_field(iter, Position, 0);
	const Rotation *rot  = ecs_field(iter, Rotation, 1);
	const Size *size = ecs_field(iter, Size, 2);
	const Center *center = ecs_field(iter, Center, 3);
	Handle *handle = ecs_field(iter, Handle, 4);
	Space *space = ecs_field(iter, Space, 7);

	for (int i = 0; i < iter->count; i++) {
		b2BodyDef body_def = b2DefaultBodyDef();
		body_def.userData = user_data(iter->entities[i]);
		body_def.type = b2_dynamicBody;
		body_def.position = (b2Vec2) {
			.x = pixels_to_meters(pos[i].x + center[i].cx),
			.y = pixels_to_meters(pos[i].y + center[i].cy) };

		b2BodyId body_id = b2CreateBody(space->world_id, &body_def);

		handle[i].body_id = body_id;

		b2Polygon dynamic_box = b2MakeBox(pixels_to_meters(size[i].width) / 2, pixels_to_meters(size[i].height) / 2);
		b2ShapeDef shape_def = b2DefaultShapeDef();
		shape_def.density = 1.0f;
		shape_def.material.friction = 0.1f;
		shape_def.enableContactEvents = true;

		b2CreatePolygonShape(body_id, &shape_def, &dynamic_box);

		// If the entity is player.
		if (ecs_field_is_set(iter, 5)) {
		}
		
		// If the entity is asteroid.
		if (ecs_field_is_set(iter, 6)) {
			b2MassData mass_data;
			mass_data.mass = 100.0f;
			mass_data.center = (b2Vec2) { 0.0f, 0.0f };
			mass_data.rotationalInertia = 50.0f;

			b2Body_SetMassData(body_id, mass_data);
		}
	}

	// printf("generate\n");
}

void control(ecs_iter_t *iter) {
	Actions *actions = ecs_field(iter, Actions, 0);
	GameState *state = ecs_field(iter, GameState, 2);

	for (int i = 0; i < iter->count; i++) {
		if (state->screen == Playing) {
			actions[i].actions = Nothing;
			
			if (IsKeyPressed(KEY_ONE)) actions[i].actions |= UseOneBullet;
			else if (IsKeyPressed(KEY_TWO)) actions[i].actions |= UseTwoBullets;
	
			if (IsKeyDown(KEY_A))		actions[i].actions |= MoveLeft;
			if (IsKeyDown(KEY_D))		actions[i].actions |= MoveRight;
			if (IsKeyDown(KEY_UP))		actions[i].actions |= MoveForward;
			if (IsKeyDown(KEY_DOWN))	actions[i].actions |= MoveBackward;
			if (IsKeyDown(KEY_LEFT))	actions[i].actions |= TurnLeft;
			if (IsKeyDown(KEY_RIGHT))	actions[i].actions |= TurnRight;
			
			if (IsKeyPressed(KEY_MINUS))		actions[i].actions |= MinimizeSpeed;
			else if (IsKeyPressed(KEY_EQUAL))	actions[i].actions |= MaximizeSpeed;

			if (IsKeyDown(KEY_Q))		actions[i].actions |= DecreaseSpeed;
			else if (IsKeyDown(KEY_E))	actions[i].actions |= IncreaseSpeed;
			
			if (IsKeyDown(KEY_SPACE))	actions[i].actions |= Brake;
			if (IsKeyDown(KEY_W))		actions[i].actions |= Shoot;

			if (IsKeyDown(KEY_LEFT_CONTROL)) {
				if (IsKeyDown(KEY_I)) actions[i].actions |= ZoomIn;
				if (IsKeyDown(KEY_O)) actions[i].actions |= ZoomOut;

				if (IsKeyPressed(KEY_F)) {
					if (state->fullscren) actions[i].actions |= FullscreenOff;
					else actions[i].actions |= FullscreenOn;
				}
			}
		}
	}

	// printf("control\n");
}

void actions(ecs_iter_t *iter) {
	const Handle *handle = ecs_field(iter, Handle, 0);
	const Actions *actions = ecs_field(iter, Actions, 1);
	Weapon *weapon  = ecs_field(iter, Weapon, 2);
	Ship *ship = ecs_field(iter, Ship, 3);
	GameState *state = ecs_field(iter, GameState, 5);

	if (state->screen == Playing) {
		for (int i = 0; i < iter->count; i++) {
			Action action = actions[i].actions;
			b2BodyId body_id = handle[i].body_id;

			float impulse = 3.90625 * b2Body_GetMass(body_id);
			float angular_impulse = 0.7510417 * b2Body_GetMass(body_id);

			b2Rot rotation = b2Body_GetRotation(body_id);
			b2Vec2 vec_left = b2RotateVector(rotation, (b2Vec2) { -impulse, 0.0f });
			b2Vec2 vec_right = b2RotateVector(rotation, (b2Vec2) { impulse, 0.0f });
			b2Vec2 vec_forward = b2RotateVector(rotation, (b2Vec2) { 0.0f, -impulse });
			b2Vec2 vec_backward = b2RotateVector(rotation, (b2Vec2) { 0.0f, impulse });

			if (action & UseOneBullet)  weapon[i].kind = OneBullet;
			if (action & UseTwoBullets) weapon[i].kind = TwoBullets;
			if (action & MoveLeft)		b2Body_ApplyLinearImpulseToCenter(body_id, vec_left, true);
			if (action & MoveRight)		b2Body_ApplyLinearImpulseToCenter(body_id, vec_right, true);
			if (action & MoveForward)	b2Body_ApplyLinearImpulseToCenter(body_id, vec_forward, true);
			if (action & MoveBackward)  b2Body_ApplyLinearImpulseToCenter(body_id, vec_backward, true);
			if (action & TurnLeft)		b2Body_ApplyAngularImpulse(body_id, -angular_impulse, true);
			if (action & TurnRight)		b2Body_ApplyAngularImpulse(body_id, angular_impulse, true);
			if (action & MinimizeSpeed) ship[i].speed = 0;
			if (action & MaximizeSpeed) ship[i].speed = 500;
			if (action & DecreaseSpeed && ship[i].speed > 0) ship[i].speed--;
			if (action & IncreaseSpeed && ship[i].speed < 50) ship[i].speed++;
			if (action & FullscreenOn)  { ToggleBorderlessWindowed(); state->fullscren = true; }
			if (action & FullscreenOff) { ToggleBorderlessWindowed(); state->fullscren = false; }
			
			if (action & ZoomIn && state->zoom < 2.0 && zoom_allowed(state->scaled)) {
				state->zoom += 0.1;
				state->scaled = clock();
			}
			
			if (action & ZoomOut && state->zoom > 1.0 && zoom_allowed(state->scaled)) {
				state->zoom -= 0.1;
				state->scaled = clock();
			}

			if (action & Brake) {
				float linear_damping = b2Body_GetLinearDamping(body_id);
				float angular_damping = b2Body_GetAngularDamping(body_id);
		
				if (linear_damping < 100) b2Body_SetLinearDamping(body_id, linear_damping * 1.2f + 0.5f);
				if (angular_damping < 100) b2Body_SetAngularDamping(body_id, angular_damping * 1.2f + 0.5f);
			} else {
				b2Vec2 vec_velocity = b2Body_GetLinearVelocity(body_id);
				float linear_velocity = fmaxf(fabsf(vec_velocity.x), fabsf(vec_velocity.y));
				float angular_velocity = fabsf(b2Body_GetAngularVelocity(body_id));
				float linear_factor = linear_velocity >= 200.0f ? linear_velocity : 0.0f;
				float angular_factor = angular_velocity >= 10.0f ? angular_velocity : 0.0f;

				b2Body_SetLinearDamping(body_id, (50 + linear_factor - ship->speed) / 10.0f );
				b2Body_SetAngularDamping(body_id, (50 + angular_factor - ship->speed) / 10.0f);
			}

			ship[i].tracing = action & ( MoveForward | MoveBackward | MoveLeft | MoveRight );
		}
	}

	// printf("actions\n");
}

void physics(ecs_iter_t *iter) {
	ecs_world_t *world = iter->world;

	GameState *state = ecs_field(iter, GameState, 0);
	Space *space = ecs_field(iter, Space, 1);
	
	if (state->screen == Playing) {
		b2World_Step(space->world_id, TIME_STEP, SUB_STEP_COUNT);

		b2ContactEvents events = b2World_GetContactEvents(space->world_id);

		for (int j = 0; j < events.beginCount; j++) {
			b2ContactBeginTouchEvent *event = events.beginEvents + j;
			b2BodyId body_id_a = b2Shape_GetBody(event->shapeIdA);
			b2BodyId body_id_b = b2Shape_GetBody(event->shapeIdB);

			ecs_entity_t *entity_a = b2Body_GetUserData(body_id_a);
			ecs_entity_t *entity_b = b2Body_GetUserData(body_id_b);

			if (ecs_is_valid(world, *entity_a) && ecs_is_valid(world, *entity_b)) {
				ecs_set(world, *entity_a, Collision, { .entity = *entity_b });
				ecs_set(world, *entity_b, Collision, { .entity = *entity_a });
			}
		}
	}

	// printf("physics\n");
}

void transformation(ecs_iter_t *iter) {
	const Handle *handle = ecs_field(iter, Handle, 0);
	Position *pos = ecs_field(iter, Position, 1);
	Rotation *rot = ecs_field(iter, Rotation, 2);
	const Center *center = ecs_field(iter, Center, 3);
	GameState *state = ecs_field(iter, GameState, 5);
	
	if (state->screen == Playing) {
		for (int i = 0; i < iter->count; i++) {
			b2Vec2 position = b2Body_GetPosition(handle[i].body_id);
			b2Rot rotation = b2Body_GetRotation(handle[i].body_id);

			pos[i].x = meters_to_pixels(position.x) - center[i].cx;
			pos[i].y = meters_to_pixels(position.y) - center[i].cy;
			rot[i].angle = b2Rot_GetAngle(rotation);

			// If the entity is player.
			if (ecs_field_is_set(iter, 4)) {
				state->position = (Vector2) { .x = pos[i].x + center[i].cx, .y = pos[i].y + center[i].cy };
			}
		}
	}

	// printf("transformation\n");
}

void camera(ecs_iter_t *iter) {
	GameState *state = ecs_field(iter, GameState, 0);
	
	if (state->screen == Playing) {
		state->camera->zoom = state->zoom;
		state->camera->target = state->position;
		state->camera->offset = (Vector2) { .x = GetScreenWidth() / 2.0f, .y = GetScreenHeight() / 2.0f };
	}

	// printf("camera\n");
}

void draw(ecs_iter_t *iter) {
	const Position *pos = ecs_field(iter, Position, 0);
	const Rotation *rot = ecs_field(iter, Rotation, 1);
	const Sprite *sprite = ecs_field(iter, Sprite, 2);
	const Center *center = ecs_field(iter, Center, 3);
	const Size *size = ecs_field(iter, Size, 4);
	Ship *ship = ecs_field(iter, Ship, 5);
	GameState *state = ecs_field(iter, GameState, 6);
	
	if (state->screen == Playing) {
		for (int i = 0; i < iter->count; i++) {
			if (rot[i].angle == 0.0f) {
				DrawTexture(sprite[i].texture, pos[i].x, pos[i].y, WHITE);
			}
			else DrawTexturePro(
				sprite[i].texture,
				(Rectangle) {
					.x = 0.0f, .y = 0.0f,
					.width = sprite[i].texture.width, .height = sprite[i].texture.height },
				(Rectangle) {
					.x = pos[i].x + center[i].cx, .y = pos[i].y + center[i].cy,
					.width = sprite[i].texture.width, .height = sprite[i].texture.height },
				(Vector2) { .x = center[i].cx, .y = center[i].cy },
				radians_to_degrees(rot[i].angle),
				WHITE);

			// If the entity has a ship component.
			if (ecs_field_is_set(iter, 5)) {
				if (ship[i].tracing) {
					int sprite_index = (ship[i].speed - 1) / 5;
					float width = ship[i].trace.width[sprite_index];
					float height = ship[i].trace.height[sprite_index];
					Pointf position = rotate_pointf(
						pos[i].x + center[i].cx - width / 2.0f,
						pos[i].y + size[i].height,
						pos[i].x + center[i].cx,
						pos[i].y + center[i].cy,
						rot[i].angle);
		
					if (ship[i].trace.tint < 255) ship[i].trace.tint += 5;
		
					DrawTexturePro(
						ship[i].trace.texture[sprite_index],
						(Rectangle) { .x = 0.0f, .y = 0.0f, .width = width, .height = height },
						(Rectangle) { .x = position.x, .y = position.y, .width = width, .height = height },
						(Vector2)   { .x = 0.0f, .y = 0.0f },
						radians_to_degrees(rot[i].angle),
						(Color)     { .r = 255 - ship[i].trace.tint, .g = 255, .b = 255, .a = 255 });
				} else ship[i].trace.tint = 0;
			}
		}
	}

	// printf("draw\n");
}

void debug(ecs_iter_t *iter) {
	GameState *state = ecs_field(iter, GameState, 0);
	Space *space = ecs_field(iter, Space, 1);
	
	if (state->screen == Playing) {
		b2World_Draw(space->world_id, &space->debug_drawer);
	}

	// printf("debug\n");
}

b2BodyId bullet_b_new(ecs_world_t *world, b2WorldId world_id, ecs_entity_t bullet,
    Point *pos, sprite_t *sprite, const Rotation *rot) {
	b2BodyDef body_def = b2DefaultBodyDef();
	body_def.userData = user_data(bullet);
	body_def.type = b2_dynamicBody;
	body_def.position = (b2Vec2) {
		.x = pixels_to_meters(pos->x + sprite->width / 2),
		.y = pixels_to_meters(pos->y + sprite->height / 2) };
	body_def.rotation = b2MakeRot(rot->angle);
	body_def.isBullet = true;

	b2BodyId body_id = b2CreateBody(world_id, &body_def);

	b2Polygon dynamic_box = b2MakeBox(pixels_to_meters(sprite->width) / 2, pixels_to_meters(sprite->height) / 2);
	b2ShapeDef shape_def = b2DefaultShapeDef();
	shape_def.density = 1.0f;
	shape_def.material.friction = 0.01f;
	shape_def.filter.groupIndex = -1;
	shape_def.enableContactEvents = true;

	b2CreatePolygonShape(body_id, &shape_def, &dynamic_box);
	
	b2Body_ApplyLinearImpulseToCenter(
		body_id,
		b2RotateVector(b2MakeRot(degrees_to_radians(-90)),
		angle_to_vector(rot->angle, 20.0)),
		true);

	return body_id;
}

void bullet_e_new(ecs_world_t *world, b2WorldId world_id, Point *pos, sprite_t *sprite, const Rotation *rot) {
	ecs_entity_t bullet = ecs_new(world);

	ecs_add_id(world, bullet, Bullet);
	ecs_set(world, bullet, Position, { .x = pos->x, .y = pos->y });
	ecs_set(world, bullet, Size,	 { .width = sprite->width, .height = sprite->height });
	ecs_set(world, bullet, Center,	 { .cx = sprite->width / 2, .cy = sprite->height / 2 });
	ecs_set(world, bullet, Rotation, { .angle = rot->angle });
	ecs_set(world, bullet, Sprite,	 { .texture = sprite->texture });
	ecs_set(world, bullet, Handle,	 { .body_id = bullet_b_new(world, world_id, bullet, pos, sprite, rot) });
}

void shooting(ecs_iter_t *iter) {
	ecs_world_t *world = iter->world;
	sprite_t *sprite = sprite_get("bullet-a");

	const Position *position = ecs_field(iter, Position, 0);
	const Center *center = ecs_field(iter, Center, 1);
	const Rotation *rot = ecs_field(iter, Rotation, 2);
	const Size *size = ecs_field(iter, Size, 3);
	Weapon *weapon = ecs_field(iter, Weapon, 4);
	const Actions *actions = ecs_field(iter, Actions, 5);
	GameState *state = ecs_field(iter, GameState, 6);
	Space *space = ecs_field(iter, Space, 7);

	if (state->screen == Playing) {
		for (int i = 0; i < iter->count; i++) {
			if (actions[i].actions & Shoot && shot_allowed(weapon[i].shot)) {
				switch (weapon[i].kind) {
					case OneBullet: {
						Point pos = {
							.x = position[i].x + center[i].cx - sprite->width / 2,
							.y = position[i].y - sprite->height
						};

						rotate_point_in(&pos, position[i].x + center[i].cx, position[i].y + center[i].cy, rot[i].angle);
						bullet_e_new(world, space->world_id, &pos, sprite, &rot[i]);
					
						break;
					}
				
					case TwoBullets: {
						int dx = size[i].width / 3;

						Point pos1 = {
							.x = position[i].x + dx - sprite->width / 2,
							.y = position[i].y - sprite->height
						};

						Point pos2 = {
							.x = position[i].x + 2 * dx - sprite->width / 2,
							.y = position[i].y - sprite->height
						};

						rotate_point_in(&pos1, position[i].x + center[i].cx, position[i].y + center[i].cy, rot[i].angle);
						rotate_point_in(&pos2, position[i].x + center[i].cx, position[i].y + center[i].cy, rot[i].angle);
		
						bullet_e_new(world, space->world_id, &pos1, sprite, &rot[i]);
						bullet_e_new(world, space->world_id, &pos2, sprite, &rot[i]);

						break;
					}
				}

				weapon[i].shot = clock();
			}
		}
	}

	// printf("shooting\n");
}

void collisions(ecs_iter_t *iter) {
	ecs_world_t *world = iter->world;

	const Collision *collision = ecs_field(iter, Collision, 0);
	const Handle *handle = ecs_field(iter, Handle, 1);
	const Position *pos = ecs_field(iter, Position, 2);
	const Center *center = ecs_field(iter, Center, 3);
	GameState *state = ecs_field(iter, GameState, 5);
	
	if (state->screen == Playing) {
		for (int i = 0; i < iter->count; i++) {
			// If the entity is bullet.
			if (ecs_field_is_set(iter, 4)) {
				free_user_data(handle[i].body_id);
				b2DestroyBody(handle[i].body_id);
				ecs_delete(world, iter->entities[i]);

				ecs_entity_t spark = ecs_new(world);

				ecs_add_id(world, spark, Spark);
				ecs_set(world, spark, Position,	 { .x = pos[i].x + center[i].cx, .y = pos[i].y + center[i].cy });
				ecs_set(world, spark, Animation, { .frame = 0, .speed = 2, .count = 3, .textures = &sprites.spark });
			}
		}
	}

	// printf("collisions\n");
}

void effects(ecs_iter_t *iter) {
	ecs_world_t *world = iter->world;

	const Position *pos = ecs_field(iter, Position, 0);
	Animation *a = ecs_field(iter, Animation, 1);
	GameState *state = ecs_field(iter, GameState, 3);
	
	if (state->screen == Playing) {
		for (int i = 0; i < iter->count; i++) {
			// Of the entity is Spark.
			if (ecs_field_is_set(iter, 2)) {
				a[i].frame++;

				if (a[i].frame == a[i].count * a[i].speed)
					ecs_delete(world, iter->entities[i]);
				else {
					Texture2D texture = (*a[i].textures)[a[i].frame / a[i].speed];
					DrawTexture(texture, pos[i].x - texture.width / 2.0, pos[i].y - texture.height / 2.0, WHITE);
				}
			}
		}
	}

	// printf("effects\n");
}

void cleaning(ecs_iter_t *iter) {
	ecs_world_t *world = iter->world;
	
	const Position *pos = ecs_field(iter, Position, 0);
	const Size *size = ecs_field(iter, Size, 1);
	const Handle *handle = ecs_field(iter, Handle, 2);
	GameState *state = ecs_field(iter, GameState, 4);

	int screen_width = GetScreenWidth();
	int screen_height = GetScreenHeight();
	
	Rectangle rect = (Rectangle) {
		.x = state->position.x - screen_width / 2.0f,
		.y = state->position.y - screen_height / 2.0f,
		.width = screen_width,
		.height = screen_height
	};

	if (state->screen == Playing) {
		for (int i = 0; i < iter->count; i++) {
			// Of the entity is Bullet.
			if (ecs_field_is_set(iter, 3)) {
				if (is_outside_of_rect(&pos[i], &size[i], &rect)) {
					free_user_data(handle[i].body_id);
					b2DestroyBody(handle[i].body_id);
					ecs_delete(world, iter->entities[i]);
				}
			}
		}
	}

	// printf("cleaning\n");
}

void destroy(ecs_iter_t *iter) {
	sprites_destroy();
	
	while (ecs_query_next(iter)) {
		Space *space = ecs_field(iter, Space, 0);
		
		b2DestroyWorld(space->world_id);
		space->world_id = b2_nullWorldId;
	}

	// printf("destroy");
}

systems_t register_systems(ecs_world_t *world) {
	ecs_entity_t _load = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "load",
			.add = ecs_ids(ecs_dependson(EcsOnStart))
		}),
		.query.terms = {
			{ ecs_id(GameState), .src = ecs_id(GameState) }
		},
		.callback = load
	});

	ecs_entity_t _create = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "create",
			.add = ecs_ids(ecs_dependson(EcsOnStart))
		}),
		.query.terms = {
			{ ecs_id(GameState), .src = ecs_id(GameState) }
		},
		.callback = create
	});

	ecs_entity_t _generate = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "generate",
			.add = ecs_ids(ecs_dependson(EcsOnStart))
		}),
		.query.terms = {
			{ .id = ecs_id(Position), .inout = EcsIn },
			{ .id = ecs_id(Rotation), .inout = EcsIn },
			{ .id = ecs_id(Size),	  .inout = EcsIn },
			{ .id = ecs_id(Center),	  .inout = EcsIn },
			{ .id = ecs_id(Handle),	  .inout = EcsOut },
			{ .id = ecs_id(Player),	  .inout = EcsInOutNone, .oper = EcsOptional },
			{ .id = ecs_id(Asteroid), .inout = EcsInOutNone, .oper = EcsOptional },
			{ ecs_id(Space), .src = ecs_id(Space) }
		},
		.callback = generate
	});

	ecs_entity_t _control = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "control",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(Actions), .inout = EcsOut },
			{ .id = ecs_id(Player),	 .inout = EcsInOutNone },
			{ ecs_id(GameState), .src = ecs_id(GameState) }
		},
		.callback = control
	});

	ecs_entity_t _actions = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "actions",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(Handle),  .inout = EcsIn },
			{ .id = ecs_id(Actions), .inout = EcsIn },
			{ .id = ecs_id(Weapon),  .inout = EcsOut },
			{ .id = ecs_id(Ship),    .inout = EcsOut },
			{ .id = ecs_id(Player),	 .inout = EcsInOutNone },
			{ ecs_id(GameState), .src = ecs_id(GameState) }
		},
		.callback = actions
	});
	
	ecs_entity_t _physics = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "physics",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ ecs_id(GameState), .src = ecs_id(GameState) },
			{ ecs_id(Space),	 .src = ecs_id(Space) }
		},
		.callback = physics
	});

	ecs_entity_t _transformation = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "transformation",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(Handle),   .inout = EcsIn },
			{ .id = ecs_id(Position), .inout = EcsInOut },
			{ .id = ecs_id(Rotation), .inout = EcsOut },
			{ .id = ecs_id(Center),   .inout = EcsIn },
			{ .id = ecs_id(Player),	  .inout = EcsInOutNone, .oper = EcsOptional },
			{ ecs_id(GameState), .src = ecs_id(GameState) }
		},
		.callback = transformation
	});

	ecs_entity_t _camera = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "camera",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ ecs_id(GameState), .src = ecs_id(GameState) }
		},
		.callback = camera
	});

	ecs_entity_t _draw = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "draw",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(Position), .inout = EcsIn },
			{ .id = ecs_id(Rotation), .inout = EcsIn },
			{ .id = ecs_id(Sprite),	  .inout = EcsIn },
			{ .id = ecs_id(Center),   .inout = EcsIn },
			{ .id = ecs_id(Size),	  .inout = EcsIn },
			{ .id = ecs_id(Ship),	  .inout = EcsInOut, .oper = EcsOptional },
			{ ecs_id(GameState), .src = ecs_id(GameState) }
		},
		.callback = draw
	});

	ecs_entity_t _debug = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "debug",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ ecs_id(GameState), .src = ecs_id(GameState) },
			{ ecs_id(Space),	 .src = ecs_id(Space) }
		},
		.callback = debug
	});

	ecs_entity_t _shooting = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "shooting",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(Position), .inout = EcsIn },
			{ .id = ecs_id(Center),   .inout = EcsIn },
			{ .id = ecs_id(Rotation), .inout = EcsIn },
			{ .id = ecs_id(Size),	  .inout = EcsIn },
			{ .id = ecs_id(Weapon),	  .inout = EcsInOut },
			{ .id = ecs_id(Actions),  .inout = EcsIn },
			{ ecs_id(GameState), .src = ecs_id(GameState) },
			{ ecs_id(Space),	 .src = ecs_id(Space) }
		},
		.callback = shooting
	});

	ecs_entity_t _collisions = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "collisions",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(Collision), .inout = EcsIn },
			{ .id = ecs_id(Handle),    .inout = EcsIn },
			{ .id = ecs_id(Position),  .inout = EcsIn },
			{ .id = ecs_id(Center),    .inout = EcsIn },
			{ .id = ecs_id(Bullet),	   .inout = EcsInOutNone, .oper = EcsOptional },
			{ ecs_id(GameState), .src = ecs_id(GameState) }
		},
		.callback = collisions
	});

	ecs_entity_t _effects = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "effects",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(Position),  .inout = EcsIn },
			{ .id = ecs_id(Animation), .inout = EcsInOut },
			{ .id = ecs_id(Spark),	   .inout = EcsInOutNone, .oper = EcsOptional },
			{ ecs_id(GameState), .src = ecs_id(GameState) }
		},
		.callback = effects
	});

	ecs_entity_t _cleaning = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "cleaning",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(Position), .inout = EcsIn },
			{ .id = ecs_id(Size),	  .inout = EcsIn },
			{ .id = ecs_id(Handle),	  .inout = EcsIn },
			{ .id = ecs_id(Bullet),	  .inout = EcsInOutNone, .oper = EcsOptional },
			{ ecs_id(GameState), .src = ecs_id(GameState) }
		},
		.callback = cleaning
	});

	ecs_entity_t _destroy = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "destroy"
		}),
		.query.terms = {
			{ ecs_id(Space), .src = ecs_id(Space) }
		},
		.run = destroy
	});

	return (systems_t) {
		.load			= _load,
		.create			= _create,
		.generate		= _generate,
		.control		= _control,
		.actions		= _actions,
		.physics		= _physics,
		.transformation	= _transformation,
		.camera			= _camera,
		.draw			= _draw,
		.debug			= _debug,
		.shooting		= _shooting,
		.collisions		= _collisions,
		.effects		= _effects,
		.cleaning		= _cleaning,
		.destroy		= _destroy
	};
}
