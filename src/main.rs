use std::rc::Weak;

use allegro::*;
use allegro_image::*;

use flecs_ecs::prelude::*;

const DISPLAY_WIDTH: i32 = 800;
const DISPLAY_HEIGHT: i32 = 600;

struct Sprites {
	sheet: Bitmap,
	sheet_px: Bitmap,
	effect_purple: Weak<SubBitmap>,
	effect_yellow: Weak<SubBitmap>,
	enemy_a: Weak<SubBitmap>,
	enemy_b: Weak<SubBitmap>,
	enemy_c: Weak<SubBitmap>,
	enemy_d: Weak<SubBitmap>,
	enemy_e: Weak<SubBitmap>,
	meteor_detailed_large: Weak<SubBitmap>,
	meteor_detailed_small: Weak<SubBitmap>,
	meteor_large: Weak<SubBitmap>,
	meteor_small: Weak<SubBitmap>,
	meteor_square_detailed_large: Weak<SubBitmap>,
	meteor_square_detailed_small: Weak<SubBitmap>,
	meteor_square_large: Weak<SubBitmap>,
	meteor_square_small: Weak<SubBitmap>,
	satellite_a: Weak<SubBitmap>,
	satellite_b: Weak<SubBitmap>,
	satellite_c: Weak<SubBitmap>,
	satellite_d: Weak<SubBitmap>,
	ship_a: Weak<SubBitmap>,
	ship_b: Weak<SubBitmap>,
	ship_c: Weak<SubBitmap>,
	ship_d: Weak<SubBitmap>,
	ship_e: Weak<SubBitmap>,
	ship_f: Weak<SubBitmap>,
	ship_g: Weak<SubBitmap>,
	ship_h: Weak<SubBitmap>,
	ship_i: Weak<SubBitmap>,
	ship_j: Weak<SubBitmap>,
	ship_k: Weak<SubBitmap>,
	ship_l: Weak<SubBitmap>,
	ship_sides_a: Weak<SubBitmap>,
	ship_sides_b: Weak<SubBitmap>,
	ship_sides_c: Weak<SubBitmap>,
	ship_sides_d: Weak<SubBitmap>,
	star_large: Weak<SubBitmap>,
	star_medium: Weak<SubBitmap>,
	star_small: Weak<SubBitmap>,
	star_tiny: Weak<SubBitmap>,
	station_a: Weak<SubBitmap>,
	station_b: Weak<SubBitmap>,
	station_c: Weak<SubBitmap>,
	trace_thin: Vec<Bitmap>,
	trace_medium: Vec<Bitmap>,
	trace_thick: Vec<Bitmap>,
	bullet_a: Weak<SubBitmap>,
	spark: [Weak<SubBitmap>; 3]
}

struct Sprite {
    image_key: String
}

fn main() {
    let mut done = false;
    let mut redraw = true;
    let core = Core::init().unwrap();
    let timer = Timer::new(&core, 1.0 / 30.0).unwrap();
    let queue = EventQueue::new(&core).unwrap();
    let display = Display::new(&core, DISPLAY_WIDTH, DISPLAY_HEIGHT).unwrap();

    ImageAddon::init(&core).unwrap();
    core.install_keyboard().unwrap();

    queue.register_event_source(core.get_keyboard_event_source().unwrap());
    queue.register_event_source(display.get_event_source());
    queue.register_event_source(timer.get_event_source());

    let sprites = init_sprites(&core);

    timer.start();

    'exit: loop {
        match queue.wait_for_event() {
            DisplayClose {..} => done = true,
            TimerTick {..} => redraw = true,
            _ => ()
        }

        if done { break 'exit; }
        
        if redraw && queue.is_empty() {
            redraw = false;
        }
    }
}

fn init_sprites(core: &Core) -> Sprites {
    let sheet = Bitmap::load(core, "assets/spritesheet.png").unwrap();
    let sheet_px = Bitmap::load(core, "assets/spritesheet-px.png").unwrap();
    let effect_purple = sheet.create_sub_bitmap(156, 32, 32, 64).unwrap();
    let effect_yellow = sheet.create_sub_bitmap(180, 181, 32, 64).unwrap();
    let enemy_a = sheet.create_sub_bitmap(0, 420, 48, 48).unwrap();
    let enemy_b = sheet.create_sub_bitmap(100, 140, 48, 48).unwrap();
	let enemy_c = sheet.create_sub_bitmap(0, 0, 64, 32).unwrap();
	let enemy_d = sheet.create_sub_bitmap(100, 188, 48, 48).unwrap();
	let enemy_e = sheet.create_sub_bitmap(100, 332, 48, 48).unwrap();
	let meteor_detailed_large = sheet.create_sub_bitmap(144, 428, 48, 48).unwrap();
	let meteor_detailed_small = sheet.create_sub_bitmap(144, 476, 32, 32).unwrap();
	let meteor_large = sheet.create_sub_bitmap(144, 380, 48, 48).unwrap();
	let meteor_small = sheet.create_sub_bitmap(112, 0, 32, 32).unwrap();
	let meteor_square_detailed_large = sheet.create_sub_bitmap(108, 32, 48, 48).unwrap();
	let meteor_square_detailed_small = sheet.create_sub_bitmap(152, 96, 32, 32).unwrap();
	let meteor_square_large = sheet.create_sub_bitmap(52, 340, 48, 48).unwrap();
	let meteor_square_small = sheet.create_sub_bitmap(144, 0, 32, 32).unwrap();
	let satellite_a = sheet.create_sub_bitmap(0, 148, 52, 44).unwrap();
	let satellite_b = sheet.create_sub_bitmap(0, 192, 52, 52).unwrap();
	let satellite_c = sheet.create_sub_bitmap(0, 332, 52, 36).unwrap();
	let satellite_d = sheet.create_sub_bitmap(0, 296, 52, 36).unwrap();
	let ship_a = sheet.create_sub_bitmap(52, 388, 32, 24).unwrap();
	let ship_b = sheet.create_sub_bitmap(148, 341, 16, 32).unwrap();
	let ship_c = sheet.create_sub_bitmap(48, 468, 48, 32).unwrap();
	let ship_d = sheet.create_sub_bitmap(64, 0, 48, 32).unwrap();
	let ship_e = sheet.create_sub_bitmap(96, 436, 48, 48).unwrap();
	let ship_f = sheet.create_sub_bitmap(96, 388, 48, 48).unwrap();
	let ship_g = sheet.create_sub_bitmap(60, 32, 48, 48).unwrap();
	let ship_h = sheet.create_sub_bitmap(56, 92, 48, 48).unwrap();
	let ship_i = sheet.create_sub_bitmap(148, 261, 32, 48).unwrap();
	let ship_j = sheet.create_sub_bitmap(52, 292, 48, 48).unwrap();
	let ship_k = sheet.create_sub_bitmap(148, 181, 32, 48).unwrap();
	let ship_l = sheet.create_sub_bitmap(52, 244, 48, 48).unwrap();
	let ship_sides_a = sheet.create_sub_bitmap(148, 128, 42, 53).unwrap();
	let ship_sides_b = sheet.create_sub_bitmap(0, 244, 52, 52).unwrap();
	let ship_sides_c = sheet.create_sub_bitmap(0, 368, 52, 52).unwrap();
	let ship_sides_d = sheet.create_sub_bitmap(0, 468, 48, 32).unwrap();
	let star_large = sheet.create_sub_bitmap(52, 196, 48, 48).unwrap();
	let star_medium = sheet.create_sub_bitmap(52, 148, 48, 48).unwrap();
	let star_small = sheet.create_sub_bitmap(96, 484, 16, 16).unwrap();
	let star_tiny = sheet.create_sub_bitmap(112, 484, 16, 16).unwrap();
	let station_a = sheet.create_sub_bitmap(48, 420, 48, 48).unwrap();
	let station_b = sheet.create_sub_bitmap(0, 92, 56, 56).unwrap();
	let station_c = sheet.create_sub_bitmap(0, 32, 60, 60).unwrap();
    let bullet_a = sheet_px.create_sub_bitmap(13, 0, 2, 9).unwrap();

	let mut trace_thin = Vec::with_capacity(10);
	let mut trace_medium = Vec::with_capacity(10);
	let mut trace_thick = Vec::with_capacity(10);

	let thin_sprite = Bitmap::load(core, "assets/trace-thin.png").unwrap();
	let medium_sprite = Bitmap::load(core, "assets/trace-medium.png").unwrap();
	let thick_sprite = Bitmap::load(core, "assets/trace-thick.png").unwrap();

	let thin_width = thin_sprite.get_width();
	let thin_height = thin_sprite.get_height();
	let medium_width = medium_sprite.get_width();
	let medium_height = medium_sprite.get_height();
	let thick_width = thick_sprite.get_width();
	let thick_height = thick_sprite.get_height();

	for i in 0..9 {
		let dh: i32 = 45 - i * 5;

		trace_thin.push(thin_sprite.create_sub_bitmap(0, dh, thin_width, thin_height - dh).unwrap().upgrade().unwrap().to_bitmap().unwrap());
		trace_medium.push(medium_sprite.create_sub_bitmap(0, dh, medium_width, medium_height - dh).unwrap().upgrade().unwrap().to_bitmap().unwrap());
		trace_thick.push(thick_sprite.create_sub_bitmap(0, dh, thick_width, thick_height - dh).unwrap().upgrade().unwrap().to_bitmap().unwrap());
	}

    trace_thin.push(thin_sprite);
    trace_medium.push(medium_sprite);
    trace_thick.push(thick_sprite);
	
	let spark: [Weak<SubBitmap>; 3] = [
	    sheet_px.create_sub_bitmap(34, 0, 10, 8).unwrap(),
	    sheet_px.create_sub_bitmap(45, 0, 7, 8).unwrap(),
        sheet_px.create_sub_bitmap(54, 0, 9, 8).unwrap()];

    Sprites { sheet, sheet_px, effect_purple, effect_yellow, enemy_a, enemy_b, enemy_c, enemy_d, enemy_e,
        meteor_detailed_large, meteor_detailed_small, meteor_large, meteor_small, meteor_square_detailed_large,
        meteor_square_detailed_small, meteor_square_large, meteor_square_small, satellite_a, satellite_b,
        satellite_c, satellite_d, ship_a, ship_b, ship_c, ship_d, ship_e, ship_f, ship_g, ship_h, ship_i, ship_j,
        ship_k, ship_l, ship_sides_a, ship_sides_b, ship_sides_c, ship_sides_d, star_large, star_medium, star_small,
        star_tiny, station_a, station_b, station_c, trace_thin, trace_medium, trace_thick, bullet_a, spark }
}
