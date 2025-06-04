#pragma once

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <raylib.h>

#include "hashmap.h"
#include "hashmap.c"

typedef struct sprite_t {
	char* key;
	char* label;
	char* file_name;
	Texture2D texture;
	int width;
	int height;
} sprite_t;

struct sprites_t {
	Texture2D trace_thin[10];
	Texture2D trace_medium[10];
	Texture2D trace_thick[10];
	Texture2D spark[3];
} sprites;

struct hashmap *map_sprites;

int sprite_compare(const void *a, const void *b, void *data) {
	const sprite_t *sa = a;
	const sprite_t *sb = b;
	return strcmp(sa->key, sb->key);
}

uint64_t sprite_hash(const void *item, uint64_t seed0, uint64_t seed1) {
	const sprite_t *sprite = item;
	return hashmap_sip(sprite->key, strlen(sprite->key), seed0, seed1);
}

void sprite_add(char* key, Image sheet, int x, int y, int width, int height) {
	char *_key = malloc(strlen(key) + 1);
	Image image = ImageFromImage(sheet, (Rectangle) { .x = x, .y = y, .width = width, .height = height });
	Texture texture = LoadTextureFromImage(image);
	
	strcpy(_key, key);

	hashmap_set(map_sprites, &(sprite_t) {
		.key = _key,
		.texture = texture,
		.width = texture.width,
		.height = texture.height
	});

	UnloadImage(image);
}

void sprite_new(char *key, char *label, char *file_name) {
	char path[200] = "assets/";
	char *_key = malloc(strlen(key) + 1);
	char *_label = malloc(strlen(label) + 1);
	char *_file_name = malloc(strlen(file_name) + 1);

	strcpy(_key, key);
	strcpy(_label, label);
	strcpy(_file_name, file_name);
	
	Texture texture = LoadTexture(strcat(path, file_name));

	hashmap_set(map_sprites, &(sprite_t) {
		.key = _key,
		.label = _label,
		.file_name = _file_name,
		.texture = texture,
		.width = texture.width,
		.height = texture.height
	});
}

void sprites_init() {
	map_sprites = hashmap_new(sizeof(sprite_t), 40, 0, 0, sprite_hash, sprite_compare, NULL, NULL);

	Image bmp_sheet_px = LoadImage("assets/spritesheet-px.png");

	sprite_add("bullet-a", bmp_sheet_px, 13, 0, 2, 9);

	Image thin_sprite = LoadImage("assets/trace-thin.png");
	Image medium_sprite = LoadImage("assets/trace-medium.png");
	Image thick_sprite = LoadImage("assets/trace-thick.png");

	sprites.trace_thin[9] = LoadTextureFromImage(thin_sprite);
	sprites.trace_medium[9] = LoadTextureFromImage(medium_sprite);
	sprites.trace_thick[9] = LoadTextureFromImage(thick_sprite);
		
	int thin_width = thin_sprite.width;
	int thin_height = thin_sprite.height;
	int medium_width = medium_sprite.width;
	int medium_height = medium_sprite.height;
	int thick_width = thick_sprite.width;
	int thick_height = thick_sprite.height;

	for (size_t i = 0; i < 9; i++) {
		int dh = 45 - i * 5;

		Image thin = ImageFromImage(thin_sprite, (Rectangle) { .x = 0, .y = dh, .width = thin_width, .height = thin_height - dh });
		Image medium = ImageFromImage(medium_sprite, (Rectangle) { .x = 0, .y = dh, .width = medium_width, .height = medium_height - dh });
		Image thick = ImageFromImage(thick_sprite, (Rectangle) { .x = 0, .y = dh, .width = thick_width, .height = thick_height - dh });

		sprites.trace_thin[i] = LoadTextureFromImage(thin);
		sprites.trace_medium[i] = LoadTextureFromImage(medium);
		sprites.trace_thick[i] = LoadTextureFromImage(thick);

		UnloadImage(thin);
		UnloadImage(medium);
		UnloadImage(thick);
	}

	Image spark0 = ImageFromImage(bmp_sheet_px, (Rectangle) { .x = 34, .y = 0, .width = 10, .height = 8 });
	Image spark1 = ImageFromImage(bmp_sheet_px, (Rectangle) { .x = 45, .y = 0, .width = 7, .height = 8 });
	Image spark2 = ImageFromImage(bmp_sheet_px, (Rectangle) { .x = 54, .y = 0, .width = 9, .height = 8 });

	sprites.spark[0] = LoadTextureFromImage(spark0);
	sprites.spark[1] = LoadTextureFromImage(spark1);
	sprites.spark[2] = LoadTextureFromImage(spark2);

	UnloadImage(spark2);
	UnloadImage(spark1);
	UnloadImage(spark0);

	UnloadImage(thin_sprite);
	UnloadImage(medium_sprite);
	UnloadImage(thick_sprite);

	UnloadImage(bmp_sheet_px);
}

sprite_t* sprite_get(char* key) {
	return (sprite_t*)hashmap_get(map_sprites, &(sprite_t) { .key = key });
}

void sprites_destroy() {
	for (size_t i = 0; i < 3; i++) UnloadTexture(sprites.spark[i]);
	
	for (size_t i = 0; i < 10; i++) {
		UnloadTexture(sprites.trace_thick[i]);
		UnloadTexture(sprites.trace_medium[i]);
		UnloadTexture(sprites.trace_thin[i]);
	}

	size_t iter = 0;
	void *item;

	while (hashmap_iter(map_sprites, &iter, &item)) {
		const sprite_t *sprite = item;

		free(sprite->key);
		free(sprite->label);
		free(sprite->file_name);
		
		UnloadTexture(sprite->texture);
	}

	hashmap_free(map_sprites);
}
