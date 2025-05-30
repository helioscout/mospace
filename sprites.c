#pragma once

#include <stdint.h>
#include <string.h>

#include <raylib.h>

#include "hashmap.h"
#include "hashmap.c"

typedef struct sprite_t {
	char* name;
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
	return strcmp(sa->name, sb->name);
}

uint64_t sprite_hash(const void *item, uint64_t seed0, uint64_t seed1) {
	const sprite_t *sprite = item;
	return hashmap_sip(sprite->name, strlen(sprite->name), seed0, seed1);
}

void sprite_add(char* name, Image sheet, int x, int y, int width, int height) {
	Image image = ImageFromImage(sheet, (Rectangle) { .x = x, .y = y, .width = width, .height = height });
	Texture texture = LoadTextureFromImage(image);
	
	hashmap_set(map_sprites, &(sprite_t) {
		.name = name,
		.texture = texture,
		.width = texture.width,
		.height = texture.height
	});

	UnloadImage(image);
}

void sprites_init() {
	map_sprites = hashmap_new(sizeof(sprite_t), 40, 0, 0, sprite_hash, sprite_compare, NULL, NULL);

	Image bmp_sheet = LoadImage("assets/spritesheet.png");
	Image bmp_sheet_px = LoadImage("assets/spritesheet-px.png");

	sprite_add("effect-purple", bmp_sheet, 156, 32, 32, 64);
	sprite_add("effect-yellow", bmp_sheet, 180, 181, 32, 64);
	sprite_add("enemy-a", bmp_sheet, 0, 420, 48, 48);
	sprite_add("enemy-b", bmp_sheet, 100, 140, 48, 48);
	sprite_add("enemy-c", bmp_sheet, 0, 0, 64, 32);
	sprite_add("enemy-d", bmp_sheet, 100, 188, 48, 48);
	sprite_add("enemy-e", bmp_sheet, 100, 332, 48, 48);
	sprite_add("meteor-detailed-large", bmp_sheet, 144, 428, 48, 48);
	sprite_add("meteor-detailed-small", bmp_sheet, 144, 476, 32, 32);
	sprite_add("meteor-large", bmp_sheet, 144, 380, 48, 48);
	sprite_add("meteor-small", bmp_sheet, 112, 0, 32, 32);
	sprite_add("meteor-square-detailed-large", bmp_sheet, 108, 32, 48, 48);
	sprite_add("meteor-square-detailed-small", bmp_sheet, 152, 96, 32, 32);
	sprite_add("meteor-square-large", bmp_sheet, 52, 340, 48, 48);
	sprite_add("meteor-square-small", bmp_sheet, 144, 0, 32, 32);
	sprite_add("satellite-a", bmp_sheet, 0, 148, 52, 44);
	sprite_add("satellite-b", bmp_sheet, 0, 192, 52, 52);
	sprite_add("satellite-c", bmp_sheet, 0, 332, 52, 36);
	sprite_add("satellite-d", bmp_sheet, 0, 296, 52, 36);
	sprite_add("ship-a", bmp_sheet, 52, 388, 32, 24);
	sprite_add("ship-b", bmp_sheet, 148, 341, 16, 32);
	sprite_add("ship-c", bmp_sheet, 48, 468, 48, 32);
	sprite_add("ship-d", bmp_sheet, 64, 0, 48, 32);
	sprite_add("ship-e", bmp_sheet, 96, 436, 48, 48);
	sprite_add("ship-f", bmp_sheet, 96, 388, 48, 48);
	sprite_add("ship-g", bmp_sheet, 60, 32, 48, 48);
	sprite_add("ship-h", bmp_sheet, 56, 92, 48, 48);
	sprite_add("ship-i", bmp_sheet, 148, 261, 32, 48);
	sprite_add("ship-j", bmp_sheet, 52, 292, 48, 48);
	sprite_add("ship-k", bmp_sheet, 148, 181, 32, 48);
	sprite_add("ship-l", bmp_sheet, 52, 244, 48, 48);
	sprite_add("ship-sides-a", bmp_sheet, 148, 128, 42, 53);
	sprite_add("ship-sides-b", bmp_sheet, 0, 244, 52, 52);
	sprite_add("ship-sides-c", bmp_sheet, 0, 368, 52, 52);
	sprite_add("ship-sides-d", bmp_sheet, 0, 468, 48, 32);
	sprite_add("star-large", bmp_sheet, 52, 196, 48, 48);
	sprite_add("star-medium", bmp_sheet, 52, 148, 48, 48);
	sprite_add("star-small", bmp_sheet, 96, 484, 16, 16);
	sprite_add("star-tiny", bmp_sheet, 112, 484, 16, 16);
	sprite_add("station-a", bmp_sheet, 48, 420, 48, 48);
	sprite_add("station-b", bmp_sheet, 0, 92, 56, 56);
	sprite_add("station-c", bmp_sheet, 0, 32, 60, 60);
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

	for (int i = 0; i < 9; i++) {
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

	UnloadImage(bmp_sheet);
	UnloadImage(bmp_sheet_px);
}

sprite_t* sprite_get(char* name) {
	return (sprite_t*)hashmap_get(map_sprites, &(sprite_t) { .name = name });
}

void sprites_destroy() {
	for (int i = 0; i < 3; i++) UnloadTexture(sprites.spark[i]);
	
	for (int i = 0; i < 10; i++) {
		UnloadTexture(sprites.trace_thick[i]);
		UnloadTexture(sprites.trace_medium[i]);
		UnloadTexture(sprites.trace_thin[i]);
	}

	size_t iter = 0;
	void *item;

	while (hashmap_iter(map_sprites, &iter, &item)) {
		const sprite_t *sprite = item;
		UnloadTexture(sprite->texture);
	}

	hashmap_free(map_sprites);
}
