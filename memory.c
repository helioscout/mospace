#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#include <allegro5/allegro5.h>
#include <box2d/box2d.h>

#define e_types_count 3
#define c_types_count 7

enum EntityType : uint_fast64_t {
	e_bullet		= 0b0000000000000000000000000000000000000000000000000000000000000001,
	e_asteroid		= 0b0000000000000000000000000000000000000000000000000000000000000010,
	e_player		= 0b0000000000000000000000000000000000000000000000000000000000000100,
	e_any			= 0b1111111111111111111111111111111111111111111111111111111111111111
};

enum ComponentType : uint_fast64_t {
	c_position		= 0b0000000000000000000000000000000000000000000000000000000000000001,
	c_size			= 0b0000000000000000000000000000000000000000000000000000000000000010,
	c_center		= 0b0000000000000000000000000000000000000000000000000000000000000100,
	c_rotation		= 0b0000000000000000000000000000000000000000000000000000000000001000,
	c_sprite		= 0b0000000000000000000000000000000000000000000000000000000000010000,
	c_ship			= 0b0000000000000000000000000000000000000000000000000000000000100000,
	c_weapon		= 0b0000000000000000000000000000000000000000000000000000000001000000,
	c_any			= 0b1111111111111111111111111111111111111111111111111111111111111111
};

enum EntityIndex {
	i_bullet		= 0,
	i_asteroid		= 1,
	i_player		= 2
};

enum ComponentIndex {
	i_position		= 0,
	i_size			= 1,
	i_center		= 2,
	i_rotation		= 3,
	i_sprite		= 4,
	i_ship			= 5,
	i_weapon		= 6
};

enum MaxEntityCount {
	max_bullets		= 1024,
	max_asteroids	= 512,
	max_players		= 1
};

enum MaxComponentCount {
	max_positions	= max_bullets + max_asteroids,
	max_sizes		= max_bullets + max_asteroids,
	max_centers		= max_bullets + max_asteroids,
	max_rotations	= max_bullets + max_asteroids,
	max_sprites		= max_bullets + max_asteroids,
	max_ships		= max_players,
	max_weapons		= max_players
};

enum IteratorType {
	it_entities,
	it_components,
	it_query
};

enum QueryOperator : uint_fast8_t {
	op_and		= 0b00000001,
	op_or		= 0b00000010,
	op_not		= 0b00000100
};

typedef enum EntityType EntityType;
typedef enum ComponentType ComponentType;
typedef enum IteratorType IteratorType;
typedef enum QueryOperator QueryOperator;

struct Entity {
	int index;
	EntityType type;
	uint_fast64_t c_types;
	void *components[c_types_count];
	b2BodyId body_id;
};

typedef struct Entity Entity;

enum WeaponType {
	w_one_bullet = 1,
	w_two_bullets = 2
};

struct Trace {
	int width[10];
	int height[10];
	int tint;					// Trace intensity.
	ALLEGRO_BITMAP* image[10];
};

typedef enum WeaponType WeaponType;
typedef struct Trace Trace;

struct Position {
	int x;
	int y;
	int index;
	Entity* entity;
};

struct Size {
	int width;
	int height;
	int index;
	Entity* entity;
};

struct Center {
	int cx;			// Center relative x coordinate (width / 2).
	int cy;			// Center relative y coordinate (height / 2).
	int index;
	Entity* entity;
};

struct Rotation {
	float angle;	// Rotation angle in radians.
	int index;
	Entity* entity;
};

struct Sprite {
	ALLEGRO_BITMAP* image;
	int index;
	Entity* entity;
};

struct Ship {
	int speed;			// Maximum ship speed from 0 to 50 (anti-damping).
	bool tracing;		// Ship tracing sign (draw trace).
	Trace trace;
	int index;
	Entity* entity;
};

struct Weapon {
	WeaponType type;
	int index;
	Entity* entity;
};

typedef struct Position Position;
typedef struct Size Size;
typedef struct Center Center;
typedef struct Rotation Rotation;
typedef struct Sprite Sprite;
typedef struct Ship Ship;
typedef struct Weapon Weapon;

struct Query {
	void* param;
	bool (*query)(Entity *entity, void *param);
};

typedef struct Query Query;
typedef struct Iterator Iterator;

struct Iterator {
	IteratorType type;
	uint_fast64_t e_types;
	uint_fast64_t c_types;
	int q_count;
	uint_fast8_t q_ops;
	Query* (*queries)[];
	Entity* entity;
	void* component;

	int e_index;
	int c_index;
	int e_sub_index;
	int c_sub_index;
	Iterator* iter;
};

uint_fast64_t e_types_order[e_types_count] = { e_bullet, e_asteroid, e_player };
int e_max_count[e_types_count] = { max_bullets, max_asteroids, max_players };
int e_max_indexes[e_types_count] = { 0, 0, 0 };

uint_fast64_t c_types_order[c_types_count] = { c_position, c_size, c_center, c_rotation, c_sprite, c_ship, c_weapon };
int c_max_count[c_types_count] = { max_positions, max_sizes, max_centers, max_rotations, max_sprites, max_ships, max_weapons };
int c_max_indexes[c_types_count] = { 0, 0, 0, 0, 0, 0, 0 };

Entity **entities[e_types_count];
void **components[c_types_count];

void* component_set(Entity *entity, ComponentType c_type, void *c_def);
int component_get_index(void *component, ComponentType c_type);
bool component_set_index(void *component, ComponentType c_type, int index);
void components_remove(Entity *entity);

int e_index(EntityType type) {
	switch (type) {
		case e_bullet: return i_bullet;
		case e_asteroid: return i_asteroid;
		case e_player: return i_player;
		case e_any: return -1;
	}
}

int c_index(ComponentType type) {
	switch (type) {
		case c_position: return i_position;
		case c_size: return i_size;
		case c_center: return i_center;
		case c_rotation: return i_rotation;
		case c_sprite: return i_sprite;
		case c_ship: return i_ship;
		case c_weapon: return i_weapon;
		case c_any: return -1;
	}
}

void memory_init() {
	for (int i = 0; i < e_types_count; i++) entities[i] = (Entity**)malloc(e_max_count[i] * sizeof(Entity*));
	for (int i = 0; i < c_types_count; i++) components[i] = (void**)malloc(c_max_count[i] * sizeof(void*));
}

void memory_dispose() {
	for (int i = 0; i < e_types_count; i++) free(entities[i]);
	for (int i = 0; i < c_types_count; i++) free(components[i]);
}

Entity* entity_new(EntityType type) {
	int i = e_index(type);
	int index = e_max_indexes[i];

	if (index == e_max_count[i]) return NULL;
	
	Entity *entity = malloc(sizeof(Entity));

	entity->index = index;
	entity->type = type;
	entity->c_types = 0;

	entities[i][index] = entity;
	e_max_indexes[i] = index + 1;

	return entity;
}

void entity_delete(Entity *entity) {
	int i = e_index(entity->type);
	int index = e_max_indexes[i];

	if (index > 1 && entity->index < index - 1) {
		// Move last entity pointer to the place of deleting one
		// and change its index field accordingly.
		Entity *e = entities[i][index - 1];
		entities[i][entity->index] = e;
		e->index = entity->index;
	}

	e_max_indexes[i] = index - 1;

	components_remove(entity);
	free(entity);
}

void* component_add(Entity *entity, ComponentType c_type, void *c_def) {
	if (entity->c_types & c_type) return component_set(entity, c_type, c_def);

	void *component;
	int i = c_index(c_type);
	int index = c_max_indexes[i];

	if (index == c_max_count[i]) return NULL;

	switch (c_type) {
		case c_position: {
			Position *position = malloc(sizeof(Position));
			position->x = ((Position*)c_def)->x;
			position->y = ((Position*)c_def)->y;
			position->index = index;
			position->entity = entity;
			component = position;
			break;
		}

		case c_size: {
			Size *size = malloc(sizeof(Size));
			size->width = ((Size*)c_def)->width;
			size->height = ((Size*)c_def)->height;
			size->index = index;
			size->entity = entity;
			component = size;
			break;
		}

		case c_center: {
			Center *center = malloc(sizeof(Center));
			center->cx = ((Center*)c_def)->cx;
			center->cy = ((Center*)c_def)->cy;
			center->index = index;
			center->entity = entity;
			component = center;
			break;
		}

		case c_rotation: {
			Rotation *rotation = malloc(sizeof(Rotation));
			rotation->angle = ((Rotation*)c_def)->angle;
			rotation->index = index;
			rotation->entity = entity;
			component = rotation;
			break;
		}

		case c_sprite: {
			Sprite *sprite = malloc(sizeof(Sprite));
			sprite->image = ((Sprite*)c_def)->image;
			sprite->index = index;
			sprite->entity = entity;
			component = sprite;
			break;
		}

		case c_ship: {
			Ship *ship = malloc(sizeof(Ship));
			ship->speed = ((Ship*)c_def)->speed;
			ship->tracing = ((Ship*)c_def)->tracing;
			ship->trace.tint = ((Ship*)c_def)->trace.tint;
			ship->index = index;
			ship->entity = entity;
			component = ship;
			break;
		}

		case c_weapon: {
			Weapon *weapon = malloc(sizeof(Weapon));
			weapon->type = ((Weapon*)c_def)->type;
			weapon->index = index;
			weapon->entity = entity;
			component = weapon;
			break;
		}

		case c_any: return NULL;
	}

	entity->components[i] = component;
	entity->c_types = entity->c_types | c_type;
	
	components[i][index] = component;
	c_max_indexes[i] = index + 1;

	return component;
}

void* component_set(Entity *entity, ComponentType c_type, void *c_def) {
	if (entity->c_types & c_type) {
		void *component = entity->components[c_index(c_type)];
		
		switch (c_type) {
			case c_position: {
				Position *position = component;
				position->x = ((Position*)c_def)->x;
				position->y = ((Position*)c_def)->y;
				break;
			}

			case c_size: {
				Size *size = component;
				size->width = ((Size*)c_def)->width;
				size->height = ((Size*)c_def)->height;
				break;
			}

			case c_center: {
				Center *center = component;
				center->cx = ((Center*)c_def)->cx;
				center->cy = ((Center*)c_def)->cy;
				break;
			}

			case c_rotation: {
				Rotation *rotation = component;
				rotation->angle = ((Rotation*)c_def)->angle;
				break;
			}

			case c_sprite: {
				Sprite *sprite = component;
				sprite->image = ((Sprite*)c_def)->image;
				break;
			}

			case c_ship: {
				Ship *ship = component;
				ship->speed = ((Ship*)c_def)->speed;
				ship->tracing = ((Ship*)c_def)->tracing;
				ship->trace.tint = ((Ship*)c_def)->trace.tint;
				break;
			}

			case c_weapon: {
				Weapon *weapon = component;
				weapon->type = ((Weapon*)c_def)->type;
				break;
			}
				
			case c_any: return NULL;
		}

		return component;
	} else return component_add(entity, c_type, c_def);
}

bool component_remove(Entity *entity, ComponentType c_type) {
	if ((entity->c_types & c_type) == 0) return false;

	int i = c_index(c_type);
	int index = c_max_indexes[i];
	void *component = entity->components[i];
	int c_index = component_get_index(component, c_type);

	if (c_index == -1) return false;
	
	if (index > 1 && c_index < index - 1) {
		// Move last component pointer to the place of deleting one
		// and change its index field accordingly.
		void *c = components[i][index - 1];
		components[i][c_index] = c;
		component_set_index(c, c_type, c_index);
	}
	
	c_max_indexes[i] = index - 1;
	entity->c_types = entity->c_types & ~c_type;

	free(component);

	return true;
}

void* component_get(Entity *entity, ComponentType c_type) {
	return (entity->c_types & c_type) ? entity->components[c_index(c_type)] : NULL;
}

int component_get_index(void *component, ComponentType c_type) {
	switch (c_type) {
		case c_position: return ((Position*)component)->index;
		case c_size: return ((Size*)component)->index;
		case c_center: return ((Center*)component)->index;
		case c_rotation: return ((Rotation*)component)->index;
		case c_sprite: return ((Sprite*)component)->index;
		case c_ship: return ((Ship*)component)->index;
		case c_weapon: return ((Weapon*)component)->index;
		case c_any: return -1;
	}
}

bool component_set_index(void *component, ComponentType c_type, int index) {
	switch (c_type) {
		case c_position: ((Position*)component)->index = index; break;
		case c_size: ((Size*)component)->index = index; break;
		case c_center: ((Center*)component)->index = index; break;
		case c_rotation: ((Rotation*)component)->index = index; break;
		case c_sprite: ((Sprite*)component)->index = index; break;
		case c_ship: ((Ship*)component)->index = index; break;
		case c_weapon: ((Weapon*)component)->index = index; break;
		case c_any: return false;
	}

	return true;
}

Entity* component_entity(void *component, ComponentType c_type) {
	switch (c_type) {
		case c_position: return ((Position*)component)->entity;
		case c_size: return ((Size*)component)->entity;
		case c_center: return ((Center*)component)->entity;
		case c_rotation: return ((Rotation*)component)->entity;
		case c_sprite: return ((Sprite*)component)->entity;
		case c_ship: return ((Ship*)component)->entity;
		case c_weapon: return ((Weapon*)component)->entity;
		case c_any: return NULL;
	}
}

void components_remove(Entity *entity) {
	if (entity->c_types & c_position) component_remove(entity, c_position);
	if (entity->c_types & c_size) component_remove(entity, c_size);
	if (entity->c_types & c_center) component_remove(entity, c_center);
	if (entity->c_types & c_rotation) component_remove(entity, c_rotation);
	if (entity->c_types & c_sprite) component_remove(entity, c_sprite);
	if (entity->c_types & c_ship) component_remove(entity, c_ship);
	if (entity->c_types & c_weapon) component_remove(entity, c_weapon);
}

int entities_count(uint_fast64_t e_types) {
	int count = 0;

	if (e_types & e_bullet) count += e_max_indexes[e_index(e_bullet)];
	if (e_types & e_asteroid) count += e_max_indexes[e_index(e_asteroid)];
	if (e_types & e_player) count += e_max_indexes[e_index(e_player)];

	return count;
}

int components_count(uint_fast64_t c_types) {
	int count = 0;

	if (c_types & c_position) count += c_max_indexes[c_index(c_position)];
	if (c_types & c_size) count += c_max_indexes[c_index(c_size)];
	if (c_types & c_center) count += c_max_indexes[c_index(c_center)];
	if (c_types & c_rotation) count += c_max_indexes[c_index(c_rotation)];
	if (c_types & c_sprite) count += c_max_indexes[c_index(c_sprite)];
	if (c_types & c_ship) count += c_max_indexes[c_index(c_ship)];
	if (c_types & c_weapon) count += c_max_indexes[c_index(c_weapon)];

	return count;
}

Iterator entities_iter(uint_fast64_t e_types) {
	return (Iterator) {
		.type = it_entities,
		.e_types = e_types,
		.e_index = -1,
		.c_index = -1,
		.e_sub_index = -1,
		.c_sub_index = -1
	};
}

Iterator components_iter(uint_fast64_t c_types, uint_fast64_t e_types) {
	return (Iterator) {
		.type = it_components,
		.c_types = c_types,
		.e_types = e_types,
		.e_index = -1,
		.c_index = -1,
		.e_sub_index = -1,
		.c_sub_index = -1
	};
}

Iterator query_iter(uint_fast64_t e_types, int q_count, uint_fast8_t q_ops, Query* (*queries)[]) {
	return (Iterator) {
		.type = it_query,
		.e_types = e_types,
		.q_count = q_count,
		.q_ops = q_ops,
		.queries = queries,
		.iter = NULL
	};
}

bool iter_next_e_index(Iterator *iter) {
	while (iter->e_index < e_types_count - 1) {
		iter->e_index = iter->e_index + 1;

		if ((iter->e_types & e_types_order[iter->e_index]) == 0 || e_max_indexes[iter->e_index] == 0) continue;

		return true;
	}

	return false;
}

bool iter_next_c_index(Iterator *iter) {
	while (iter->c_index < c_types_count - 1) {
		iter->c_index = iter->c_index + 1;

		if ((iter->c_types & c_types_order[iter->c_index]) == 0 || c_max_indexes[iter->c_index] == 0) continue;

		return true;
	}

	return false;
}

bool iter_next(Iterator *iter) {
	if (iter->type == it_entities) {
		if (iter->e_index == -1 || iter->e_sub_index == e_max_indexes[iter->e_index] - 1) {
			if (!iter_next_e_index(iter)) return false;

			iter->e_sub_index = -1;
		}

		iter->e_sub_index = iter->e_sub_index + 1;
		iter->entity = entities[iter->e_index][iter->e_sub_index];
	} else if (iter->type == it_components) {
		if (iter->c_index == -1 || iter->c_sub_index == c_max_indexes[iter->c_index] - 1) {
			if (!iter_next_c_index(iter)) return false;

			iter->c_sub_index = -1;
		}

		iter->c_sub_index = iter->c_sub_index + 1;

		while ( (iter->component = components[iter->c_index][iter->c_sub_index]) &&
		        (component_entity(iter->component, c_types_order[iter->c_index])->type & iter->e_types) == 0 ) {
			if (iter->c_sub_index == c_max_indexes[iter->c_index] - 1) return iter_next(iter);

			iter->c_sub_index = iter->c_sub_index + 1;
		}
	} else if (iter->type == it_query) {
		if (iter->iter == NULL) {
			iter->iter = malloc(sizeof(Iterator));
			*iter->iter = entities_iter(iter->e_types);
		}

		if (!iter_next(iter->iter)) {
			free(iter->iter);
			return false;
		}

		while (true) {
			bool matches[iter->q_count];
			
			for (int i = 0; i < iter->q_count; i++) {
				Query *query = (*iter->queries)[i];
				matches[i] = query->query(iter->iter->entity, query->param);
			}

			bool match = matches[0];

			for (int i = 1; i < iter->q_count; i++) {
				if (iter->q_ops & op_and) match = match && matches[i];
				else if (iter->q_ops & op_or) match = match || matches[i];
			}

			if (iter->q_ops & op_not) match = !match;

			if (match) {
				iter->entity = iter->iter->entity;
				break;
			}

			if (!iter_next(iter->iter)) {
				free(iter->iter);
				return false;
			}
		}
	}

	return true;
}
