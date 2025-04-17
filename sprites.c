#pragma once

#include <allegro5/allegro5.h>
#include <stdint.h>
#include <string.h>
#include "hashmap.h"
#include "hashmap.c"

struct sprite_t {
	char* name;
	ALLEGRO_BITMAP* bitmap;
	int width;
	int height;
};

typedef struct sprite_t sprite_t;

struct sprites_t {
	ALLEGRO_BITMAP* trace_thin[10];
	ALLEGRO_BITMAP* trace_medium[10];
	ALLEGRO_BITMAP* trace_thick[10];
	ALLEGRO_BITMAP* spark[3];
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

void sprite_add(char* name, ALLEGRO_BITMAP *sheet, int x, int y, int width, int height) {
	ALLEGRO_BITMAP *bitmap = al_create_sub_bitmap(sheet, x, y, width, height);

	hashmap_set(map_sprites, &(sprite_t){
		.name = name,
		.bitmap = bitmap,
		.width = al_get_bitmap_width(bitmap),
		.height = al_get_bitmap_height(bitmap)
	});
}

void sprites_init() {
	map_sprites = hashmap_new(sizeof(sprite_t), 40, 0, 0, sprite_hash, sprite_compare, NULL, NULL);

	ALLEGRO_BITMAP *bmp_sheet = al_load_bitmap("assets/spritesheet.png");
	ALLEGRO_BITMAP *bmp_sheet_px = al_load_bitmap("assets/spritesheet-px.png");

	hashmap_set(map_sprites, &(sprite_t){
		.name = "sheet",
		.bitmap = bmp_sheet,
		.width = al_get_bitmap_width(bmp_sheet),
		.height = al_get_bitmap_height(bmp_sheet)
	});

	hashmap_set(map_sprites, &(sprite_t){
		.name = "sheet-px",
		.bitmap = bmp_sheet_px,
		.width = al_get_bitmap_width(bmp_sheet_px),
		.height = al_get_bitmap_height(bmp_sheet_px)
	});

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

	ALLEGRO_BITMAP* thin_sprite = sprites.trace_thin[9] = al_load_bitmap("assets/trace-thin.png");
	ALLEGRO_BITMAP* medium_sprite = sprites.trace_medium[9] = al_load_bitmap("assets/trace-medium.png");
	ALLEGRO_BITMAP* thick_sprite = sprites.trace_thick[9] = al_load_bitmap("assets/trace-thick.png");

	int thin_width = al_get_bitmap_width(thin_sprite);
	int thin_height = al_get_bitmap_height(thin_sprite);
	int medium_width = al_get_bitmap_width(medium_sprite);
	int medium_height = al_get_bitmap_height(medium_sprite);
	int thick_width = al_get_bitmap_width(thick_sprite);
	int thick_height = al_get_bitmap_height(thick_sprite);

	for (int i = 0; i < 9; i++) {
		int dh = 45 - i * 5;
		
		sprites.trace_thin[i] = al_create_sub_bitmap(thin_sprite, 0, dh, thin_width, thin_height - dh);
		sprites.trace_medium[i] = al_create_sub_bitmap(medium_sprite, 0, dh, medium_width, medium_height - dh);
		sprites.trace_thick[i] = al_create_sub_bitmap(thick_sprite, 0, dh, thick_width, thick_height - dh);
	}

	sprites.spark[0] = al_create_sub_bitmap(bmp_sheet_px, 34, 0, 10, 8);
	sprites.spark[1] = al_create_sub_bitmap(bmp_sheet_px, 45, 0, 7, 8);
	sprites.spark[2] = al_create_sub_bitmap(bmp_sheet_px, 54, 0, 9, 8);
}

sprite_t* sprite_get(char* name) {
	return (sprite_t*)hashmap_get(map_sprites, &(sprite_t){ .name = name });
}

void sprites_destroy() {
	for (int i = 0; i < 3; i++) al_destroy_bitmap(sprites.spark[i]);
	
	for (int i = 0; i < 10; i++) {
		al_destroy_bitmap(sprites.trace_thick[i]);
		al_destroy_bitmap(sprites.trace_medium[i]);
		al_destroy_bitmap(sprites.trace_thin[i]);
	}

	size_t iter = 0;
	void *item;

	while (hashmap_iter(map_sprites, &iter, &item)) {
		const sprite_t *sprite = item;
		al_destroy_bitmap(sprite->bitmap);
		printf("%s ", sprite->name);
	}

	hashmap_free(map_sprites);
}
