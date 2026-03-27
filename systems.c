#pragma once

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <raylib.h>
#include <flecs.h>
#include <box2d/box2d.h>

#include "components.c"
#include "sprites.c"
#include "helpers.c"
#include "debug.c"
#include "gui.c"
#include "db.c"

typedef struct systems_t {
	ecs_entity_t prepare;
	ecs_entity_t control;
	ecs_entity_t global_control;
	ecs_entity_t actions;
	ecs_entity_t global_actions;
	ecs_entity_t physics;
	ecs_entity_t transformation;
	ecs_entity_t camera;
	ecs_entity_t draw;
	ecs_entity_t gui;
	ecs_entity_t gui_actions;
	ecs_entity_t debug;
	ecs_entity_t shooting;
	ecs_entity_t collisions;
	ecs_entity_t effects;
	ecs_entity_t cleaning;
	ecs_entity_t destroy;
} systems_t;

void prepare(ecs_iter_t *iter) {
	ecs_world_t *world = iter->world;

	GameState *state = ecs_field(iter, GameState, 0);
	GameGui *gui = ecs_field(iter, GameGui, 1);

	b2DebugDraw	debug_drawer = b2DefaultDebugDraw();
	debug_drawer.drawShapes = true;
	debug_drawer.DrawPolygonFcn = draw_polygon;
	debug_drawer.DrawSolidPolygonFcn = draw_solid_polygon;
	debug_drawer.DrawSegmentFcn = draw_segment;
	debug_drawer.DrawPointFcn = draw_point;

	ecs_singleton_set(world, Space, {
		.world_id = b2_nullWorldId,
		.debug_drawer = debug_drawer
	});

	gui->layout(state, gui);
}

void control(ecs_iter_t *iter) {
	Actions *actions = ecs_field(iter, Actions, 0);
	GameState *state = ecs_field(iter, GameState, 2);

	for (size_t i = 0; i < (size_t)iter->count; i++) {
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
			}
		}
	}
}

void global_control(ecs_iter_t *iter) {
	GameState *state = ecs_field(iter, GameState, 0);

	state->actions = Nothing;

	if (IsKeyDown(KEY_LEFT_CONTROL)) {
		if (IsKeyPressed(KEY_F)) {
			if (state->fullscren) state->actions |= FullscreenOff;
			else state->actions |= FullscreenOn;
		}
	}

	if (IsKeyPressed(KEY_ESCAPE)) state->actions |= Escape;

	if (IsWindowResized()) {
		state->size = (Vector2) { .x = GetScreenWidth(), .y = GetScreenHeight() };
		state->actions |= Resize;
	}
}

void actions(ecs_iter_t *iter) {
	const Handle *handle   = ecs_field(iter, Handle, 0);
	const Actions *actions = ecs_field(iter, Actions, 1);
	Weapon *weapon		   = ecs_field(iter, Weapon, 2);
	Ship *ship			   = ecs_field(iter, Ship, 3);
	GameState *state	   = ecs_field(iter, GameState, 5);
	GameGui *gui		   = ecs_field(iter, GameGui, 6);

	for (size_t i = 0; i < (size_t)iter->count; i++) {
		Action action = actions[i].actions;
		
		if (state->screen == Playing) {
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
			if (action & MaximizeSpeed) ship[i].speed = 50;
			if (action & DecreaseSpeed && ship[i].speed > 0) ship[i].speed--;
			if (action & IncreaseSpeed && ship[i].speed < 50) ship[i].speed++;
			
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
}

void global_actions(ecs_iter_t *iter) {
	GameState *state = ecs_field(iter, GameState, 0);
	GameGui *gui	 = ecs_field(iter, GameGui, 1);

	if (state->actions & FullscreenOn)  {
		ToggleBorderlessWindowed();
	
		state->fullscren = true;
		state->size = (Vector2) { .x = GetScreenWidth(), .y = GetScreenHeight() };
	
		if (state->screen == Menu) gui->layout(state, gui);
	}

	if (state->actions & FullscreenOff) {
		ToggleBorderlessWindowed();

		state->fullscren = false;
		state->size = (Vector2) { .x = GetScreenWidth(), .y = GetScreenHeight() };
	
		if (state->screen == Menu) gui->layout(state, gui);
	}

	if (state->actions & Resize && state->screen == Menu) {
		gui->layout(state, gui);
	}

	if (state->actions & Escape) {
		if (state->screen == Playing) {
			state->position = (Vector2) { .x = GetScreenWidth() / 2.0f, .y = GetScreenHeight() / 2.0f };
			state->screen = Menu;

			/* We need to adjust gui position to current screen size. */
			gui->layout(state, gui);
		}
		else if (state->screen == Menu && state->loaded) state->screen = Playing;
	}
}

void physics(ecs_iter_t *iter) {
	ecs_world_t *world = iter->world;

	GameState *state = ecs_field(iter, GameState, 0);
	Space *space = ecs_field(iter, Space, 1);
	
	if (state->screen == Playing) {
		b2World_Step(space->world_id, TIME_STEP, SUB_STEP_COUNT);

		b2ContactEvents events = b2World_GetContactEvents(space->world_id);

		for (size_t j = 0; j < (size_t)events.beginCount; j++) {
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
}

void transformation(ecs_iter_t *iter) {
	const Handle *handle = ecs_field(iter, Handle, 0);
	Position *pos = ecs_field(iter, Position, 1);
	Rotation *rot = ecs_field(iter, Rotation, 2);
	const Center *center = ecs_field(iter, Center, 3);
	GameState *state = ecs_field(iter, GameState, 5);
	
	if (state->screen == Playing) {
		for (size_t i = 0; i < (size_t)iter->count; i++) {
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
}

void camera(ecs_iter_t *iter) {
	GameState *state = ecs_field(iter, GameState, 0);
	
	state->camera->zoom   = state->screen == Playing ? state->zoom : 1.0f;
	state->camera->target = state->screen == Playing ? state->position : (Vector2) { .x = 0.0f, .y = 0.0f };
	state->camera->offset = state->screen == Playing
		? (Vector2) { .x = GetScreenWidth() / 2.0f, .y = GetScreenHeight() / 2.0f }
		: (Vector2) { .x = 0.0f, .y = 0.0f };
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
		for (size_t i = 0; i < (size_t)iter->count; i++) {
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
}

void gui(ecs_iter_t *iter) {
	GameState *state = ecs_field(iter, GameState, 0);
	GameGui *gui = ecs_field(iter, GameGui, 1);

	gui_draw(state, gui);
}

void gui_actions(ecs_iter_t *iter) {
	ecs_world_t *world = iter->world;

	GameState *state = ecs_field(iter, GameState, 0);
	GameGui *gui = ecs_field(iter, GameGui, 1);
	Space *space = ecs_field(iter, Space, 2);

	if (state->actions & LoadWorld) {
		uint64_t id = gui->gui_menu->world_ids[gui->gui_menu->world_index];
		/* Clear all user data attched to rigid bodies. */
		clear_user_data(world);
		/* Clear all entities. */
		ecs_iter_t iter = ecs_each(world, Position);

		while (ecs_each_next(&iter)) {
			for (size_t i = 0; i < (size_t)iter.count; ++i) {
				ecs_clear(world, iter.entities[i]);
			}
		}

		/* Recreate physics world. */
		if (B2_IS_NON_NULL(space->world_id)) b2DestroyWorld(space->world_id);

		b2WorldDef world_def = b2DefaultWorldDef();
		world_def.gravity = (b2Vec2) { 0.0f, 0.0f };

		space->world_id = b2CreateWorld(&world_def);

		load_world(state, world, space->world_id, id);

		state->screen = Playing;
		state->loaded = true;
	} else if (state->actions & ExitGame) SHOULD_EXIT_GAME = true;
}

void debug(ecs_iter_t *iter) {
	GameState *state = ecs_field(iter, GameState, 0);
	Space *space = ecs_field(iter, Space, 1);
	
	if (state->screen == Playing) {
		b2World_Draw(space->world_id, &space->debug_drawer);
	}
}

b2BodyId bullet_b_new([[maybe_unused]] ecs_world_t *world, b2WorldId world_id, ecs_entity_t bullet,
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
		for (size_t i = 0; i < (size_t)iter->count; i++) {
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
}

void collisions(ecs_iter_t *iter) {
	ecs_world_t *world = iter->world;

	[[maybe_unused]] const Collision *collision = ecs_field(iter, Collision, 0);
	const Handle *handle = ecs_field(iter, Handle, 1);
	const Position *pos = ecs_field(iter, Position, 2);
	const Center *center = ecs_field(iter, Center, 3);
	GameState *state = ecs_field(iter, GameState, 5);
	
	if (state->screen == Playing) {
		for (size_t i = 0; i < (size_t)iter->count; i++) {
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
}

void effects(ecs_iter_t *iter) {
	ecs_world_t *world = iter->world;

	const Position *pos = ecs_field(iter, Position, 0);
	Animation *a = ecs_field(iter, Animation, 1);
	GameState *state = ecs_field(iter, GameState, 3);
	
	if (state->screen == Playing) {
		for (size_t i = 0; i < (size_t)iter->count; i++) {
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
		for (size_t i = 0; i < (size_t)iter->count; i++) {
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
}

void destroy(ecs_iter_t *iter) {
	ecs_world_t *world = iter->world;

	sprites_destroy();
	
	while (ecs_query_next(iter)) {
		GameState *state = ecs_field(iter, GameState, 0);
		GameGui *gui = ecs_field(iter, GameGui, 1);
		Space *space = ecs_field(iter, Space, 2);

		if (B2_IS_NON_NULL(space->world_id)) {
			clear_user_data(world);
			b2DestroyWorld(space->world_id);
			space->world_id = b2_nullWorldId;
		}

		for (size_t i = 0; i < gui->gui_menu->worlds_count; ++i) {
			free(gui->gui_menu->world_labels[i]);
		}

		free(state->map->label);
		free(state->map);

		free(gui->w_menu);
		free(gui->gui_menu);
	}
}

systems_t register_systems(ecs_world_t *world) {
	ecs_entity_t _prepare = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "prepare",
			.add = ecs_ids(ecs_dependson(EcsOnStart))
		}),
		.query.terms = {
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) },
			{ .id = ecs_id(GameGui),   .src.id = ecs_id(GameGui) }
		},
		.callback = prepare
	});

	ecs_entity_t _control = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "control",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(Actions),   .inout = EcsOut },
			{ .id = ecs_id(Player),	   .inout = EcsInOutNone },
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) }
		},
		.callback = control
	});

	ecs_entity_t _global_control = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "global_control",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) }
		},
		.callback = global_control
	});

	ecs_entity_t _actions = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "actions",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(Handle),    .inout = EcsIn },
			{ .id = ecs_id(Actions),   .inout = EcsIn },
			{ .id = ecs_id(Weapon),    .inout = EcsOut },
			{ .id = ecs_id(Ship),      .inout = EcsOut },
			{ .id = ecs_id(Player),	   .inout = EcsInOutNone },
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) },
			{ .id = ecs_id(GameGui),   .src.id = ecs_id(GameGui) }
		},
		.callback = actions
	});
	
	ecs_entity_t _global_actions = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "global_actions",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) },
			{ .id = ecs_id(GameGui),   .src.id = ecs_id(GameGui) }
		},
		.callback = global_actions
	});

	ecs_entity_t _physics = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "physics",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) },
			{ .id = ecs_id(Space),	   .src.id = ecs_id(Space) }
		},
		.callback = physics
	});

	ecs_entity_t _transformation = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "transformation",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(Handle),    .inout = EcsIn },
			{ .id = ecs_id(Position),  .inout = EcsInOut },
			{ .id = ecs_id(Rotation),  .inout = EcsOut },
			{ .id = ecs_id(Center),    .inout = EcsIn },
			{ .id = ecs_id(Player),	   .inout = EcsInOutNone, .oper = EcsOptional },
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) }
		},
		.callback = transformation
	});

	ecs_entity_t _camera = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "camera",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) }
		},
		.callback = camera
	});

	ecs_entity_t _draw = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "draw",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(Position),  .inout = EcsIn },
			{ .id = ecs_id(Rotation),  .inout = EcsIn },
			{ .id = ecs_id(Sprite),	   .inout = EcsIn },
			{ .id = ecs_id(Center),    .inout = EcsIn },
			{ .id = ecs_id(Size),	   .inout = EcsIn },
			{ .id = ecs_id(Ship),	   .inout = EcsInOut, .oper = EcsOptional },
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) }
		},
		.callback = draw
	});

	ecs_entity_t _gui = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "gui",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) },
			{ .id = ecs_id(GameGui),   .src.id = ecs_id(GameGui) }
		},
		.callback = gui
	});

	ecs_entity_t _gui_actions = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "gui_actions",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) },
			{ .id = ecs_id(GameGui),   .src.id = ecs_id(GameGui) },
			{ .id = ecs_id(Space),	   .src.id = ecs_id(Space) }
		},
		.callback = gui_actions
	});

	ecs_entity_t _debug = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "debug",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) },
			{ .id = ecs_id(Space),	   .src.id = ecs_id(Space) }
		},
		.callback = debug
	});

	ecs_entity_t _shooting = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "shooting",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(Position),  .inout = EcsIn },
			{ .id = ecs_id(Center),    .inout = EcsIn },
			{ .id = ecs_id(Rotation),  .inout = EcsIn },
			{ .id = ecs_id(Size),	   .inout = EcsIn },
			{ .id = ecs_id(Weapon),	   .inout = EcsInOut },
			{ .id = ecs_id(Actions),   .inout = EcsIn },
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) },
			{ .id = ecs_id(Space),	   .src.id = ecs_id(Space) }
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
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) }
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
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) }
		},
		.callback = effects
	});

	ecs_entity_t _cleaning = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "cleaning",
			.add = ecs_ids(ecs_dependson(EcsOnUpdate))
		}),
		.query.terms = {
			{ .id = ecs_id(Position),  .inout = EcsIn },
			{ .id = ecs_id(Size),	   .inout = EcsIn },
			{ .id = ecs_id(Handle),	   .inout = EcsIn },
			{ .id = ecs_id(Bullet),	   .inout = EcsInOutNone, .oper = EcsOptional },
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) }
		},
		.callback = cleaning
	});

	ecs_entity_t _destroy = ecs_system(world, {
		.entity = ecs_entity(world, {
			.name = "destroy"
		}),
		.query.terms = {
			{ .id = ecs_id(GameState), .src.id = ecs_id(GameState) },
			{ .id = ecs_id(GameGui),   .src.id = ecs_id(GameGui) },
			{ .id = ecs_id(Space),     .src.id = ecs_id(Space) }
		},
		.run = destroy,
		.immediate = true
	});

	return (systems_t) {
		.prepare		= _prepare,
		.control		= _control,
		.global_control = _global_control,
		.actions		= _actions,
		.global_actions = _global_actions,
		.physics		= _physics,
		.transformation	= _transformation,
		.camera			= _camera,
		.draw			= _draw,
		.gui			= _gui,
		.gui_actions	= _gui_actions,
		.debug			= _debug,
		.shooting		= _shooting,
		.collisions		= _collisions,
		.effects		= _effects,
		.cleaning		= _cleaning,
		.destroy		= _destroy
	};
}
