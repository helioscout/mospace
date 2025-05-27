#pragma once

#include <raylib.h>
#include <box2d/box2d.h>
#include <flecs.h>

enum WeaponType {
	w_one_bullet = 1,
	w_two_bullets = 2
};

struct Trace {
	int width[10];
	int height[10];
	int tint;					// Trace intensity.
	Texture2D texture[10];
};

typedef enum WeaponType WeaponType;
typedef struct Trace Trace;

struct Position {
	int x;
	int y;
};

struct Size {
	int width;
	int height;
};

struct Center {
	int cx;			// Center relative x coordinate (width / 2).
	int cy;			// Center relative y coordinate (height / 2).
};

struct Rotation {
	float angle;	// Rotation angle in radians.
};

struct Sprite {
	Texture2D texture;
};

struct Ship {
	int speed;			// Maximum ship speed from 0 to 50 (anti-damping).
	bool tracing;		// Ship tracing sign (draw trace).
	Trace trace;
};

struct Weapon {
	WeaponType type;
};

struct Animation {
	int frame;						// Current game loop frame since animation start.
	int speed;						// Game loop frames count per animation frame.
	int count;						// Animation frames count.
	Texture2D (*textures)[];		// Animation frames images (sprites).
};

struct Physics {
	b2BodyId body_id;
};

typedef struct Position Position;
typedef struct Size Size;
typedef struct Center Center;
typedef struct Rotation Rotation;
typedef struct Sprite Sprite;
typedef struct Ship Ship;
typedef struct Weapon Weapon;
typedef struct Animation Animation;
typedef struct Physics Physics;

ECS_COMPONENT_DECLARE(Sprite);
ECS_COMPONENT_DECLARE(Size);
ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Center);
ECS_COMPONENT_DECLARE(Rotation);
ECS_COMPONENT_DECLARE(Weapon);
ECS_COMPONENT_DECLARE(Ship);
ECS_COMPONENT_DECLARE(Physics);
ECS_COMPONENT_DECLARE(Animation);

ECS_TAG_DECLARE(Bullet);
ECS_TAG_DECLARE(Asteroid);
ECS_TAG_DECLARE(Spark);

void register_components(ecs_world_t *world) {
	ECS_COMPONENT_DEFINE(world, Sprite);
	ECS_COMPONENT_DEFINE(world, Size);
	ECS_COMPONENT_DEFINE(world, Position);
	ECS_COMPONENT_DEFINE(world, Center);
	ECS_COMPONENT_DEFINE(world, Rotation);
	ECS_COMPONENT_DEFINE(world, Weapon);
	ECS_COMPONENT_DEFINE(world, Ship);
	ECS_COMPONENT_DEFINE(world, Physics);
	ECS_COMPONENT_DEFINE(world, Animation);
	
	ECS_TAG_DEFINE(world, Bullet);
	ECS_TAG_DEFINE(world, Asteroid);
	ECS_TAG_DEFINE(world, Spark);
}
