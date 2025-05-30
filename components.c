#pragma once

#include <time.h>
#include <stdbool.h>

#include <raylib.h>
#include <box2d/box2d.h>
#include <flecs.h>

typedef enum Action : uint_fast64_t {
	Nothing =		0b0000000000000000000000000000000000000000000000000000000000000000,
	UseOneBullet =	0b0000000000000000000000000000000000000000000000000000000000000001,
	UseTwoBullets =	0b0000000000000000000000000000000000000000000000000000000000000010,
	Weapon3 =		0b0000000000000000000000000000000000000000000000000000000000000100,
	Weapon4 =		0b0000000000000000000000000000000000000000000000000000000000001000,
	Weapon5 =		0b0000000000000000000000000000000000000000000000000000000000010000,
	Weapon6 =		0b0000000000000000000000000000000000000000000000000000000000100000,
	Weapon7 =		0b0000000000000000000000000000000000000000000000000000000001000000,
	Weapon8 =		0b0000000000000000000000000000000000000000000000000000000010000000,
	Weapon9 =		0b0000000000000000000000000000000000000000000000000000000100000000,
	MoveForward =	0b0000000000000000000000000000000000000000000000000000001000000000,
	MoveBackward =	0b0000000000000000000000000000000000000000000000000000010000000000,
	MoveLeft =		0b0000000000000000000000000000000000000000000000000000100000000000,
	MoveRight =		0b0000000000000000000000000000000000000000000000000001000000000000,
	TurnLeft =		0b0000000000000000000000000000000000000000000000000010000000000000,
	TurnRight =		0b0000000000000000000000000000000000000000000000000100000000000000,
	IncreaseSpeed =	0b0000000000000000000000000000000000000000000000001000000000000000,
	DecreaseSpeed =	0b0000000000000000000000000000000000000000000000010000000000000000,
	MaximizeSpeed =	0b0000000000000000000000000000000000000000000000100000000000000000,
	MinimizeSpeed =	0b0000000000000000000000000000000000000000000001000000000000000000,
	Brake =			0b0000000000000000000000000000000000000000000010000000000000000000,
	Shoot =			0b0000000000000000000000000000000000000000000100000000000000000000,
	ZoomIn =		0b0000000000000000000000000000000000000000001000000000000000000000,
	ZoomOut =		0b0000000000000000000000000000000000000000010000000000000000000000,
	FullscreenOn =	0b0000000000000000000000000000000000000000100000000000000000000000,
	FullscreenOff =	0b0000000000000000000000000000000000000001000000000000000000000000
} Action;

typedef enum GameScreen {
	Menu,
	Playing,
	Paused,
	Over
} GameScreen;

typedef enum WeaponKind {
	OneBullet = 1,
	TwoBullets = 2
} WeaponKind;

typedef struct Trace {
	int width[10];
	int height[10];
	int tint;					// Trace intensity.
	Texture2D texture[10];
} Trace;

typedef struct Actions {
	Action actions;
} Actions;

typedef struct Position {
	int x;
	int y;
} Position;

typedef struct Size {
	int width;
	int height;
} Size;

typedef struct Center {
	int cx;			// Center relative x coordinate (width / 2).
	int cy;			// Center relative y coordinate (height / 2).
} Center;

typedef struct Rotation {
	float angle;	// Rotation angle in radians.
} Rotation;

typedef struct Sprite {
	Texture2D texture;
} Sprite;

typedef struct Ship {
	int speed;			// Maximum ship speed from 0 to 50 (anti-damping).
	bool tracing;		// Ship tracing sign (draw trace).
	Trace trace;
} Ship;

typedef struct Weapon {
	WeaponKind kind;
	clock_t shot;
} Weapon;

typedef struct Animation {
	int frame;						// Current game loop frame since animation start.
	int speed;						// Game loop frames count per animation frame.
	int count;						// Animation frames count.
	Texture2D (*textures)[];		// Animation frames images (sprites).
} Animation;

typedef struct Handle {
	b2BodyId body_id;
} Handle;

typedef struct Collision {
	ecs_entity_t entity;
} Collision;

typedef struct Space {
	b2WorldId world_id;
	b2DebugDraw debug_drawer;
} Space;

typedef struct GameState {
	Camera2D *camera;
	GameScreen screen;
	Vector2 position;
	bool fullscren;
	float zoom;
	clock_t scaled;
} GameState;

ECS_COMPONENT_DECLARE(Actions);
ECS_COMPONENT_DECLARE(GameState);
ECS_COMPONENT_DECLARE(Sprite);
ECS_COMPONENT_DECLARE(Size);
ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Center);
ECS_COMPONENT_DECLARE(Rotation);
ECS_COMPONENT_DECLARE(Weapon);
ECS_COMPONENT_DECLARE(Ship);
ECS_COMPONENT_DECLARE(Handle);
ECS_COMPONENT_DECLARE(Collision);
ECS_COMPONENT_DECLARE(Animation);
ECS_COMPONENT_DECLARE(Space);

ECS_TAG_DECLARE(Player);
ECS_TAG_DECLARE(Bullet);
ECS_TAG_DECLARE(Asteroid);
ECS_TAG_DECLARE(Spark);

void register_components(ecs_world_t *world) {
	ECS_COMPONENT_DEFINE(world, Actions);
	ECS_COMPONENT_DEFINE(world, GameState);
	ECS_COMPONENT_DEFINE(world, Sprite);
	ECS_COMPONENT_DEFINE(world, Size);
	ECS_COMPONENT_DEFINE(world, Position);
	ECS_COMPONENT_DEFINE(world, Center);
	ECS_COMPONENT_DEFINE(world, Rotation);
	ECS_COMPONENT_DEFINE(world, Weapon);
	ECS_COMPONENT_DEFINE(world, Ship);
	ECS_COMPONENT_DEFINE(world, Handle);
	ECS_COMPONENT_DEFINE(world, Collision);
	ECS_COMPONENT_DEFINE(world, Animation);
	ECS_COMPONENT_DEFINE(world, Space);

	ECS_TAG_DEFINE(world, Player);
	ECS_TAG_DEFINE(world, Bullet);
	ECS_TAG_DEFINE(world, Asteroid);
	ECS_TAG_DEFINE(world, Spark);
}
