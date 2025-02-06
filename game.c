#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <time.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>

#include <box2d/box2d.h>

#define KEY_SEEN 1
#define KEY_RELEASED 2
#define SHIP_SPEED 1

const int display_width = 800;
const int display_height = 600;
int display_center_x = display_width / 2;
int display_center_y = display_height / 2;
unsigned char key[ALLEGRO_KEY_MAX];
float scaling_factor = 0.1f;
float time_step = 1.0f / 60.0f;
int sub_step_count = 4;
int bullets_interval = 100;			// Interval between bullet shots in milliseconds.
clock_t shot_time = 0;				// Last shot time.

b2WorldId world_id;

enum WeaponType {
	ONE_BULLET = 1,
	TWO_BULLETS = 2
};

struct Sprites {
	ALLEGRO_BITMAP* sheet;
	ALLEGRO_BITMAP* sheet_px;
	ALLEGRO_BITMAP* effect_purple;
	ALLEGRO_BITMAP* effect_yellow;
	ALLEGRO_BITMAP* enemy_a;
	ALLEGRO_BITMAP* enemy_b;
	ALLEGRO_BITMAP* enemy_c;
	ALLEGRO_BITMAP* enemy_d;
	ALLEGRO_BITMAP* enemy_e;
	ALLEGRO_BITMAP* meteor_detailed_large;
	ALLEGRO_BITMAP* meteor_detailed_small;
	ALLEGRO_BITMAP* meteor_large;
	ALLEGRO_BITMAP* meteor_small;
	ALLEGRO_BITMAP* meteor_square_detailed_large;
	ALLEGRO_BITMAP* meteor_square_detailed_small;
	ALLEGRO_BITMAP* meteor_square_large;
	ALLEGRO_BITMAP* meteor_square_small;
	ALLEGRO_BITMAP* satellite_a;
	ALLEGRO_BITMAP* satellite_b;
	ALLEGRO_BITMAP* satellite_c;
	ALLEGRO_BITMAP* satellite_d;
	ALLEGRO_BITMAP* ship_a;
	ALLEGRO_BITMAP* ship_b;
	ALLEGRO_BITMAP* ship_c;
	ALLEGRO_BITMAP* ship_d;
	ALLEGRO_BITMAP* ship_e;
	ALLEGRO_BITMAP* ship_f;
	ALLEGRO_BITMAP* ship_g;
	ALLEGRO_BITMAP* ship_h;
	ALLEGRO_BITMAP* ship_i;
	ALLEGRO_BITMAP* ship_j;
	ALLEGRO_BITMAP* ship_k;
	ALLEGRO_BITMAP* ship_l;
	ALLEGRO_BITMAP* ship_sides_a;
	ALLEGRO_BITMAP* ship_sides_b;
	ALLEGRO_BITMAP* ship_sides_c;
	ALLEGRO_BITMAP* ship_sides_d;
	ALLEGRO_BITMAP* star_large;
	ALLEGRO_BITMAP* star_medium;
	ALLEGRO_BITMAP* star_small;
	ALLEGRO_BITMAP* star_tiny;
	ALLEGRO_BITMAP* station_a;
	ALLEGRO_BITMAP* station_b;
	ALLEGRO_BITMAP* station_c;
	ALLEGRO_BITMAP* trace_thin[10];
	ALLEGRO_BITMAP* trace_medium[10];
	ALLEGRO_BITMAP* trace_thick[10];
	ALLEGRO_BITMAP* bullet_a;
} sprites;

struct Point {
	int x, y;
};

struct Ship {
	int x, y, width, height;
	int cx, cy;					// Ship center relative coordinates (width|height / 2).
	float angle;				// Ship rotation angle in radians.
	int speed;					// Maximum ship speed from 0 to 50 (anti-damping).
	ALLEGRO_BITMAP* sprite;
};

struct Trace {
	int width[10];
	int height[10];
	int tint_factor;			// Trace intensity.
	ALLEGRO_BITMAP* sprite[10];
};

struct Player {
	struct Ship ship;
	struct Trace trace;
	enum WeaponType weapon_type;
	b2BodyId body_id;
	bool tracing;
} player = {
	.ship = {
		.x = 0,
		.y = 0,
		.width = 0,
		.height = 0,
		.cx = 0,
		.cy = 0,
		.angle = 0.0f,
		.speed = 50,
		.sprite = NULL },
	.trace = {
		.tint_factor = 0 },
	.weapon_type = ONE_BULLET,
	.body_id = b2_nullBodyId,
	.tracing = false };

struct Bullet {
	int start_x, start_y, x, y, width, height, cx, cy;
	float angle;
	b2BodyId body_id;
	ALLEGRO_BITMAP* sprite;
};

struct Bullet bullets[1000];
int bullet_count = 0;

void log_msg(const char* message, const char* description);
void init(bool value, const char* description);
void* create(void* value, const char* description);
ALLEGRO_BITMAP* grab_sprite(int x, int y, int width, int height);
ALLEGRO_BITMAP* grab_sprite_px(int x, int y, int width, int height);
float pixels_to_meters(int pixels);
int meters_to_pixels(float meters);
struct Point rotate_point(int x, int y, int cx, int cy, float angle);
void rotate_point_in(struct Point *point, int cx, int cy, float angle);
b2Vec2 angle_to_vector(float angle, float scale);
float degrees_to_radians(int degrees);

void init_sprites() {
	sprites.sheet = create(al_load_bitmap("assets/spritesheet.png"), "spritesheet");
	sprites.sheet_px = create(al_load_bitmap("assets/spritesheet-px.png"), "spritesheet-px");

	sprites.effect_purple = grab_sprite(156, 32, 32, 64);
	sprites.effect_yellow = grab_sprite(180, 181, 32, 64);
	sprites.enemy_a = grab_sprite(0, 420, 48, 48);
	sprites.enemy_b = grab_sprite(100, 140, 48, 48);
	sprites.enemy_c = grab_sprite(0, 0, 64, 32);
	sprites.enemy_d = grab_sprite(100, 188, 48, 48);
	sprites.enemy_e = grab_sprite(100, 332, 48, 48);
	sprites.meteor_detailed_large = grab_sprite(144, 428, 48, 48);
	sprites.meteor_detailed_small = grab_sprite(144, 476, 32, 32);
	sprites.meteor_large = grab_sprite(144, 380, 48, 48);
	sprites.meteor_small = grab_sprite(112, 0, 32, 32);
	sprites.meteor_square_detailed_large = grab_sprite(108, 32, 48, 48);
	sprites.meteor_square_detailed_small = grab_sprite(152, 96, 32, 32);
	sprites.meteor_square_large = grab_sprite(52, 340, 48, 48);
	sprites.meteor_square_small = grab_sprite(144, 0, 32, 32);
	sprites.satellite_a = grab_sprite(0, 148, 52, 44);
	sprites.satellite_b = grab_sprite(0, 192, 52, 52);
	sprites.satellite_c = grab_sprite(0, 332, 52, 36);
	sprites.satellite_d = grab_sprite(0, 296, 52, 36);
	sprites.ship_a = grab_sprite(52, 388, 32, 24);
	sprites.ship_b = grab_sprite(148, 341, 16, 32);
	sprites.ship_c = grab_sprite(48, 468, 48, 32);
	sprites.ship_d = grab_sprite(64, 0, 48, 32);
	sprites.ship_e = grab_sprite(96, 436, 48, 48);
	sprites.ship_f = grab_sprite(96, 388, 48, 48);
	sprites.ship_g = grab_sprite(60, 32, 48, 48);
	sprites.ship_h = grab_sprite(56, 92, 48, 48);
	sprites.ship_i = grab_sprite(148, 261, 32, 48);
	sprites.ship_j = grab_sprite(52, 292, 48, 48);
	sprites.ship_k = grab_sprite(148, 181, 32, 48);
	sprites.ship_l = grab_sprite(52, 244, 48, 48);
	sprites.ship_sides_a = grab_sprite(148, 128, 42, 53);
	sprites.ship_sides_b = grab_sprite(0, 244, 52, 52);
	sprites.ship_sides_c = grab_sprite(0, 368, 52, 52);
	sprites.ship_sides_d = grab_sprite(0, 468, 48, 32);
	sprites.star_large = grab_sprite(52, 196, 48, 48);
	sprites.star_medium = grab_sprite(52, 148, 48, 48);
	sprites.star_small = grab_sprite(96, 484, 16, 16);
	sprites.star_tiny = grab_sprite(112, 484, 16, 16);
	sprites.station_a = grab_sprite(48, 420, 48, 48);
	sprites.station_b = grab_sprite(0, 92, 56, 56);
	sprites.station_c = grab_sprite(0, 32, 60, 60);
	sprites.bullet_a = grab_sprite_px(13, 0, 2, 9);

	ALLEGRO_BITMAP* thin_sprite = sprites.trace_thin[9] = create(al_load_bitmap("assets/trace-thin.png"), "trace thin sprite");
	ALLEGRO_BITMAP* medium_sprite = sprites.trace_medium[9] = create(al_load_bitmap("assets/trace-medium.png"), "trace medium sprite");
	ALLEGRO_BITMAP* thick_sprite = sprites.trace_thick[9] = create(al_load_bitmap("assets/trace-thick.png"), "trace thick sprite");

	int thin_width = al_get_bitmap_width(thin_sprite);
	int thin_height = al_get_bitmap_height(thin_sprite);
	int medium_width = al_get_bitmap_width(medium_sprite);
	int medium_height = al_get_bitmap_height(medium_sprite);
	int thick_width = al_get_bitmap_width(thick_sprite);
	int thick_height = al_get_bitmap_height(thick_sprite);

	for (int i = 0; i < 9; i++) {
		int dh = 45 - i * 5;
		
		sprites.trace_thin[i] = create(al_create_sub_bitmap(thin_sprite, 0, dh, thin_width, thin_height - dh), "trace thin subimage");
		sprites.trace_medium[i] = create(al_create_sub_bitmap(medium_sprite, 0, dh, medium_width, medium_height - dh), "trace medium subimage");
		sprites.trace_thick[i] = create(al_create_sub_bitmap(thick_sprite, 0, dh, thick_width, thick_height - dh), "trace thick subimage");
	}
}

void destroy_sprites() {
	for (int i = 0; i < 10; i++) {
		al_destroy_bitmap(sprites.trace_thick[i]);
		al_destroy_bitmap(sprites.trace_medium[i]);
		al_destroy_bitmap(sprites.trace_thin[i]);
	}
		
	al_destroy_bitmap(sprites.bullet_a);
	al_destroy_bitmap(sprites.station_c);
	al_destroy_bitmap(sprites.station_b);
	al_destroy_bitmap(sprites.station_a);
	al_destroy_bitmap(sprites.star_tiny);
	al_destroy_bitmap(sprites.star_small);
	al_destroy_bitmap(sprites.star_medium);
	al_destroy_bitmap(sprites.star_large);
	al_destroy_bitmap(sprites.ship_sides_d);
	al_destroy_bitmap(sprites.ship_sides_c);
	al_destroy_bitmap(sprites.ship_sides_b);
	al_destroy_bitmap(sprites.ship_sides_a);
	al_destroy_bitmap(sprites.ship_l);
	al_destroy_bitmap(sprites.ship_k);
	al_destroy_bitmap(sprites.ship_j);
	al_destroy_bitmap(sprites.ship_i);
	al_destroy_bitmap(sprites.ship_h);
	al_destroy_bitmap(sprites.ship_g);
	al_destroy_bitmap(sprites.ship_f);
	al_destroy_bitmap(sprites.ship_e);
	al_destroy_bitmap(sprites.ship_d);
	al_destroy_bitmap(sprites.ship_c);
	al_destroy_bitmap(sprites.ship_b);
	al_destroy_bitmap(sprites.ship_a);
	al_destroy_bitmap(sprites.satellite_d);
	al_destroy_bitmap(sprites.satellite_c);
	al_destroy_bitmap(sprites.satellite_b);
	al_destroy_bitmap(sprites.satellite_a);
	al_destroy_bitmap(sprites.meteor_square_small);
	al_destroy_bitmap(sprites.meteor_square_large);
	al_destroy_bitmap(sprites.meteor_square_detailed_small);
	al_destroy_bitmap(sprites.meteor_square_detailed_large);
	al_destroy_bitmap(sprites.meteor_small);
	al_destroy_bitmap(sprites.meteor_large);
	al_destroy_bitmap(sprites.meteor_detailed_small);
	al_destroy_bitmap(sprites.meteor_detailed_large);
	al_destroy_bitmap(sprites.enemy_e);
	al_destroy_bitmap(sprites.enemy_d);
	al_destroy_bitmap(sprites.enemy_c);
	al_destroy_bitmap(sprites.enemy_b);
	al_destroy_bitmap(sprites.enemy_a);
	al_destroy_bitmap(sprites.effect_yellow);
	al_destroy_bitmap(sprites.effect_purple);
	al_destroy_bitmap(sprites.sheet_px);
	al_destroy_bitmap(sprites.sheet);
}

void init_keyboard() {
	memset(key, 0, sizeof(key));
}

void init_player() {
	player.ship.sprite = sprites.ship_a;
	player.ship.width = al_get_bitmap_width(player.ship.sprite);
	player.ship.height = al_get_bitmap_height(player.ship.sprite);
	player.ship.cx = player.ship.width / 2;
	player.ship.cy = player.ship.height / 2;

	for (int i = 0; i < 10; i++) {
		player.trace.sprite[i] = sprites.trace_thin[i];
		player.trace.width[i] = al_get_bitmap_width(player.trace.sprite[i]);
		player.trace.height[i] = al_get_bitmap_height(player.trace.sprite[i]);
	}
}

void init_physics() {
	b2WorldDef world_def = b2DefaultWorldDef();
	world_def.gravity = (b2Vec2){ 0.0f, 0.0f };

	world_id = b2CreateWorld(&world_def);

	b2BodyDef body_def = b2DefaultBodyDef();
	body_def.type = b2_dynamicBody;
	body_def.position = (b2Vec2){ pixels_to_meters(display_center_x), pixels_to_meters(display_center_y) };

	player.body_id = b2CreateBody(world_id, &body_def);

	b2Polygon dynamic_box = b2MakeBox(pixels_to_meters(player.ship.width) / 2, pixels_to_meters(player.ship.height) / 2);
	b2ShapeDef shape_def = b2DefaultShapeDef();
	shape_def.density = 1.0f;
	shape_def.friction = 0.1f;

	b2CreatePolygonShape(player.body_id, &shape_def, &dynamic_box);
}

void destroy_physics() {
	b2DestroyWorld(world_id);
	world_id = b2_nullWorldId;
}

void process_keyboard(ALLEGRO_EVENT* event) {
	switch (event->type) {
		case ALLEGRO_EVENT_TIMER:
			for (int i = 0; i < ALLEGRO_KEY_MAX; i++) key[i] &= KEY_SEEN;
			break;

		case ALLEGRO_EVENT_KEY_DOWN:
			key[event->keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
			break;
			
		case ALLEGRO_EVENT_KEY_UP:
			key[event->keyboard.keycode] &= KEY_RELEASED;
			break;
	}
}

void init_bullet(struct Bullet* bullet) {
	b2BodyDef body_def = b2DefaultBodyDef();
	body_def.type = b2_dynamicBody;
	body_def.position = (b2Vec2){ pixels_to_meters(bullet->start_x + bullet->width / 2), pixels_to_meters(bullet->start_y + bullet->height / 2) };
	body_def.rotation = b2MakeRot(bullet->angle);
	body_def.isBullet = true;

	bullet->body_id = b2CreateBody(world_id, &body_def);

	b2Polygon dynamic_box = b2MakeBox(pixels_to_meters(bullet->width) / 2, pixels_to_meters(bullet->height) / 2);
	b2ShapeDef shape_def = b2DefaultShapeDef();
	shape_def.density = 1.0f;
	shape_def.friction = 0.01f;
	shape_def.filter.groupIndex = -1;

	b2CreatePolygonShape(bullet->body_id, &shape_def, &dynamic_box);
	
	b2Body_ApplyLinearImpulseToCenter(bullet->body_id, b2RotateVector(b2MakeRot(degrees_to_radians(-90)), angle_to_vector(bullet->angle, 20.0)), true);
}

bool bullet_shot_allowed() {
	if (!shot_time) return true;

	double duration = 1000.0 * (clock() - shot_time) / CLOCKS_PER_SEC;
	return duration >= bullets_interval;
}

void player_shot() {
	int bullet_width = al_get_bitmap_width(sprites.bullet_a);
	int bullet_height = al_get_bitmap_height(sprites.bullet_a);
	
	if (player.weapon_type == ONE_BULLET) {
		if (!bullet_shot_allowed()) return;
		
		struct Point pos = {
			.x = player.ship.x + player.ship.cx - bullet_width / 2,
			.y = player.ship.y - bullet_height
		};

		rotate_point_in(&pos, player.ship.x + player.ship.cx, player.ship.y + player.ship.cy, player.ship.angle);
		
		bullets[bullet_count] = (struct Bullet) {
			.sprite = sprites.bullet_a,
			.start_x = pos.x,
			.start_y = pos.y,
			.x = pos.x,
			.y = pos.y,
			.width = bullet_width,
			.height = bullet_height,
			.cx = bullet_width / 2,
			.cy = bullet_height / 2,
			.angle = player.ship.angle
		};
		
		init_bullet(&bullets[bullet_count]);
		bullet_count++;

		shot_time = clock();
	} else if (player.weapon_type == TWO_BULLETS) {
		if (!bullet_shot_allowed()) return;

		int dx = player.ship.width / 3;

		struct Point pos1 = {
			.x = player.ship.x + dx - bullet_width / 2,
			.y = player.ship.y - bullet_height
		};

		struct Point pos2 = {
			.x = player.ship.x + 2 * dx - bullet_width / 2,
			.y = player.ship.y - bullet_height
		};

		rotate_point_in(&pos1, player.ship.x + player.ship.cx, player.ship.y + player.ship.cy, player.ship.angle);
		rotate_point_in(&pos2, player.ship.x + player.ship.cx, player.ship.y + player.ship.cy, player.ship.angle);
		
		bullets[bullet_count] = (struct Bullet) {
			.sprite = sprites.bullet_a,
			.start_x = pos1.x,
			.start_y = pos1.y,
			.x = pos1.x,
			.y = pos1.y,
			.width = bullet_width,
			.height = bullet_height,
			.cx = bullet_width / 2,
			.cy = bullet_height / 2,
			.angle = player.ship.angle
		};
		
		init_bullet(&bullets[bullet_count]);
		bullet_count++;

		bullets[bullet_count] = (struct Bullet) {
			.sprite = sprites.bullet_a,
			.start_x = pos2.x,
			.start_y = pos2.y,
			.x = pos2.x,
			.y = pos2.y,
			.width = bullet_width,
			.height = bullet_height,
			.cx = bullet_width / 2,
			.cy = bullet_height / 2,
			.angle = player.ship.angle
		};
		
		init_bullet(&bullets[bullet_count]);
		bullet_count++;

		shot_time = clock();
	}
}

void process_player() {
	if (key[ALLEGRO_KEY_1]) player.weapon_type = ONE_BULLET;
	else if (key[ALLEGRO_KEY_2]) player.weapon_type = TWO_BULLETS;
	
	if (key[ALLEGRO_KEY_A]) b2Body_ApplyLinearImpulseToCenter(player.body_id, b2RotateVector(b2Body_GetRotation(player.body_id), (b2Vec2){ -30.0f, 0.0f }), true);
	if (key[ALLEGRO_KEY_D]) b2Body_ApplyLinearImpulseToCenter(player.body_id, b2RotateVector(b2Body_GetRotation(player.body_id), (b2Vec2){ 30.0f, 0.0f }), true);
	if (key[ALLEGRO_KEY_UP]) b2Body_ApplyLinearImpulseToCenter(player.body_id, b2RotateVector(b2Body_GetRotation(player.body_id), (b2Vec2){ 0.0f, -30.0f }), true);
	if (key[ALLEGRO_KEY_DOWN]) b2Body_ApplyLinearImpulseToCenter(player.body_id, b2RotateVector(b2Body_GetRotation(player.body_id), (b2Vec2){ 0.0f, 30.0f }), true);
	if (key[ALLEGRO_KEY_RIGHT]) b2Body_ApplyAngularImpulse(player.body_id, 5, true);
	if (key[ALLEGRO_KEY_LEFT]) b2Body_ApplyAngularImpulse(player.body_id, -5, true);

	if (key[ALLEGRO_KEY_Q]) {
		if (player.ship.speed > 0) player.ship.speed--;
	} else if (key[ALLEGRO_KEY_E]) {
		if (player.ship.speed < 50) player.ship.speed++;
	} else if (key[ALLEGRO_KEY_MINUS]) player.ship.speed = 0;
	else if (key[ALLEGRO_KEY_EQUALS]) player.ship.speed = 50;

	// Emergency braking.
	if (key[ALLEGRO_KEY_SPACE]) {
		float linear_damping = b2Body_GetLinearDamping(player.body_id);
		float angular_damping = b2Body_GetAngularDamping(player.body_id);
		
		if (linear_damping < 100) b2Body_SetLinearDamping(player.body_id, linear_damping * 1.2f + 0.5f);
		if (angular_damping < 100) b2Body_SetAngularDamping(player.body_id, angular_damping * 1.2f + 0.5f);
	} else {
		b2Body_SetLinearDamping(player.body_id, (50 - player.ship.speed) / 10.0f );
		b2Body_SetAngularDamping(player.body_id, (50 - player.ship.speed) / 10.0f);
	}

	if (key[ALLEGRO_KEY_W]) player_shot();
	
	player.tracing = key[ALLEGRO_KEY_UP] || key[ALLEGRO_KEY_DOWN] || key[ALLEGRO_KEY_A] || key[ALLEGRO_KEY_D];
}

void process_physics() {
	b2World_Step(world_id, time_step, sub_step_count);
	
	b2Vec2 position = b2Body_GetPosition(player.body_id);
	b2Rot rotation = b2Body_GetRotation(player.body_id);

	player.ship.x = meters_to_pixels(position.x) - player.ship.cx;
	player.ship.y = meters_to_pixels(position.y) - player.ship.cy;
	player.ship.angle = b2Rot_GetAngle(rotation);

	for (int i = 0; i < bullet_count; i++) {
		struct Bullet *bullet = &bullets[i];

		b2Vec2 position = b2Body_GetPosition(bullet->body_id);
		b2Rot rotation = b2Body_GetRotation(bullet->body_id);

		bullet->x = meters_to_pixels(position.x) - bullet->cx;
		bullet->y = meters_to_pixels(position.y) - bullet->cy;
		bullet->angle = b2Rot_GetAngle(rotation);
	}
}

void draw_player() {
	if (player.ship.angle == 0.0f) al_draw_bitmap(player.ship.sprite, player.ship.x, player.ship.y, 0);
	else al_draw_rotated_bitmap(player.ship.sprite, player.ship.cx, player.ship.cy, player.ship.x + player.ship.cx, player.ship.y + player.ship.cy, player.ship.angle, 0);

	if (player.tracing) {
		int sprite_index = (player.ship.speed - 1) / 5;
		
		if (player.trace.tint_factor < 255) player.trace.tint_factor += 5;

		al_draw_tinted_rotated_bitmap(
			player.trace.sprite[sprite_index],
			al_map_rgb(255 - player.trace.tint_factor, 255, 255),
			player.trace.width[sprite_index] / 2.0f,
			-player.ship.cy,
			player.ship.x + player.ship.cx,
			player.ship.y + player.ship.cy,
			player.ship.angle,
			0);
	} else player.trace.tint_factor = 0;
}

void draw_bullets() {
	for (int i = 0; i < bullet_count; i++) {
		struct Bullet *bullet = &bullets[i];

		if (bullet->angle == 0.0f) al_draw_bitmap(bullet->sprite, bullet->x, bullet->y, 0);
		else al_draw_rotated_bitmap(bullet->sprite, bullet->cx, bullet->cy, bullet->x + bullet->cx, bullet->y + bullet->cy, bullet->angle, 0);
	}
}

int main() {
	bool done = false;
	bool redraw = true;
	ALLEGRO_EVENT event;

	init(al_init(), "allegro");
	init(al_install_keyboard(), "keyboard");
	init(al_init_image_addon(), "image addon");

	ALLEGRO_TIMER* timer = create(al_create_timer(1.0 / 30.0), "timer");
	ALLEGRO_EVENT_QUEUE* queue = create(al_create_event_queue(), "queue");
	ALLEGRO_DISPLAY* display = create(al_create_display(display_width, display_height), "display");

	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_timer_event_source(timer));

	init_sprites();
	init_keyboard();
	init_player();
	init_physics();
	
	al_start_timer(timer);

	while (true) {
		al_wait_for_event(queue, &event);

		switch (event.type) {
			case ALLEGRO_EVENT_TIMER:
				process_player();
				process_physics();
				redraw = true;
				break;
				
			case ALLEGRO_EVENT_DISPLAY_CLOSE: done = true; break;
		}

		if (done) break;

		process_keyboard(&event);

		if (redraw && al_is_event_queue_empty(queue)) {
			al_clear_to_color(al_map_rgb(0, 0, 0));
			draw_player();
			draw_bullets();
			al_flip_display();

			redraw = false;
		}
	}

	al_destroy_display(display);
	al_destroy_timer(timer);
	al_destroy_event_queue(queue);
	destroy_sprites();
	destroy_physics();
	
	return EXIT_SUCCESS;
}

void init(bool value, const char* description) {
	if (value) return;

	log_msg("couldn't initialize %s\n", description);
	exit(EXIT_FAILURE);
}

void* create(void* value, const char* description) {
	if (value) return value;

	log_msg("couldn't initialize %s\n", description);
	exit(EXIT_FAILURE);
}

ALLEGRO_BITMAP* grab_sprite(int x, int y, int width, int height) {
	return (ALLEGRO_BITMAP*)create(al_create_sub_bitmap(sprites.sheet, x, y, width, height), "sprite");
}

ALLEGRO_BITMAP* grab_sprite_px(int x, int y, int width, int height) {
	return (ALLEGRO_BITMAP*)create(al_create_sub_bitmap(sprites.sheet_px, x, y, width, height), "sprite-px");
}

float pixels_to_meters(int pixels) { return pixels * scaling_factor; }
int meters_to_pixels(float meters) { return (int)(meters / scaling_factor); }

struct Point rotate_point(int x, int y, int cx, int cy, float angle) {
	return (struct Point) {
		.x = lroundf(cosf(angle) * (x - cx) - sinf(angle) * (y - cy) + cx),
		.y = lroundf(sinf(angle) * (x - cx) + cosf(angle) * (y - cy) + cy)
	};
}

void rotate_point_in(struct Point *point, int cx, int cy, float angle) {
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

void log_msg(const char* message, const char* description) {
	FILE* f = fopen("log.txt", "a");
	fprintf(f, message, description);
	fclose(f);
}
