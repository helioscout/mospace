use std::rc::Weak;
use std::rc::Rc;
use std::collections::HashMap;

use allegro::*;
use allegro_image::*;
use flecs_ecs::prelude::*;
use rapier2d::prelude::*;

const DISPLAY_WIDTH: i32 = 800;
const DISPLAY_HEIGHT: i32 = 600;
const DISPLAY_CENTER_X: i32 = DISPLAY_WIDTH / 2;
const DISPLAY_CENTER_Y: i32 = DISPLAY_HEIGHT / 2;
const KEY_SEEN: u32 = 1;
const KEY_RELEASED: u32 = 2;

#[derive(Debug)]
enum WeaponKind {
	OneBullet = 1,
	TwoBullets = 2
}

struct Sprites<'a> {
	sheet: Bitmap,
	sheet_px: Bitmap,
	sprites: HashMap<&'a str, Weak<SubBitmap>>,
	traces: HashMap<&'a str, Vec<Bitmap>>,
	spark: [Weak<SubBitmap>; 3]
}

#[derive(Debug)]
struct Trace<'a> {
    key: &'a str,
    tint: u8
}

#[derive(Debug, Component)]
struct Sprite<'a> {
    key: &'a str
}

#[derive(Debug, Component)]
struct Size {
    width: i32,
    height: i32
}

#[derive(Debug, Component)]
struct Position {
    x: f32,
    y: f32
}

#[derive(Debug, Component)]
struct Center {
    cx: f32,       // Center relative x coordinate (width / 2).
    cy: f32        // Center relative y coordinate (height / 2).
}

#[derive(Debug, Component)]
struct Rotation {
    angle: f32    // Rotation angle in radians.
}

#[derive(Debug, Component)]
struct Ship<'a> {
    speed: i32,       // Maximum ship speed from 0 to 50 (anti-damping).
    tracing: bool,    // Ship tracing sign (draw trace).
    trace: Trace<'a>
}

#[derive(Debug, Component)]
struct Weapon {
    kind: WeaponKind
}

#[derive(Debug, Component)]
struct Handle {
    handle: RigidBodyHandle
}

fn main() {
    let mut key: HashMap<KeyCode, u32> = HashMap::from([
        (KeyCode::Up, 0),
        (KeyCode::Down, 0),
        (KeyCode::Left, 0),
        (KeyCode::Right, 0),
        (KeyCode::A, 0),
        (KeyCode::D, 0),
        (KeyCode::Q, 0),
        (KeyCode::E, 0),
        (KeyCode::W, 0),
        (KeyCode::_1, 0),
        (KeyCode::_2, 0),
        (KeyCode::Minus, 0),
        (KeyCode::Equals, 0),
        (KeyCode::Space, 0)
    ]);
    
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
    let world = World::new();
    let mut space = RigidBodySet::new();
    let mut colliders = ColliderSet::new();
    let gravity = vector![0.0, 0.0];
    let integration_parameters = IntegrationParameters::default();
    let mut physics_pipeline = PhysicsPipeline::new();
    let mut island_manager = IslandManager::new();
    let mut broad_phase = DefaultBroadPhase::new();
    let mut narrow_phase = NarrowPhase::new();
    let mut impulse_joint_set = ImpulseJointSet::new();
    let mut multibody_joint_set = MultibodyJointSet::new();
    let mut ccd_solver = CCDSolver::new();
    let mut query_pipeline = QueryPipeline::new();
    let physics_hooks = ();
    let event_handler = ();
    
    init_player(&world, &sprites);
    init_physics(&world, &mut space, &mut colliders);

    timer.start();

    'exit: loop {
        match queue.wait_for_event() {
            DisplayClose {..} => done = true,
            KeyDown { keycode, ..} => {
                if key.contains_key(&keycode) { key.insert(keycode, KEY_SEEN | KEY_RELEASED); }
            },
            KeyUp { keycode, ..} => {
                if key.contains_key(&keycode) { key.insert(keycode, key.get(&keycode).unwrap() & KEY_RELEASED); }
            },
            TimerTick {..} => {
                process_player(&key, &world, &mut space);
                
                physics_pipeline.step(
                    &gravity,
                    &integration_parameters,
                    &mut island_manager,
                    &mut broad_phase,
                    &mut narrow_phase,
                    &mut space,
                    &mut colliders,
                    &mut impulse_joint_set,
                    &mut multibody_joint_set,
                    &mut ccd_solver,
                    Some(&mut query_pipeline),
                    &physics_hooks,
                    &event_handler
                );

                process_physics(&world, &space);

                key.insert(KeyCode::Up, key.get(&KeyCode::Up).unwrap() & KEY_SEEN);
                key.insert(KeyCode::Down, key.get(&KeyCode::Down).unwrap() & KEY_SEEN);
                key.insert(KeyCode::Left, key.get(&KeyCode::Left).unwrap() & KEY_SEEN);
                key.insert(KeyCode::Right, key.get(&KeyCode::Right).unwrap() & KEY_SEEN);
                key.insert(KeyCode::A, key.get(&KeyCode::A).unwrap() & KEY_SEEN);
                key.insert(KeyCode::D, key.get(&KeyCode::D).unwrap() & KEY_SEEN);
                key.insert(KeyCode::Q, key.get(&KeyCode::Q).unwrap() & KEY_SEEN);
                key.insert(KeyCode::E, key.get(&KeyCode::E).unwrap() & KEY_SEEN);
                key.insert(KeyCode::W, key.get(&KeyCode::W).unwrap() & KEY_SEEN);
                key.insert(KeyCode::_1, key.get(&KeyCode::_1).unwrap() & KEY_SEEN);
                key.insert(KeyCode::_2, key.get(&KeyCode::_2).unwrap() & KEY_SEEN);
                key.insert(KeyCode::Minus, key.get(&KeyCode::Minus).unwrap() & KEY_SEEN);
                key.insert(KeyCode::Equals, key.get(&KeyCode::Equals).unwrap() & KEY_SEEN);
                key.insert(KeyCode::Space, key.get(&KeyCode::Space).unwrap() & KEY_SEEN);
                
                redraw = true
            },
            _ => ()
        }

        if done { break 'exit; }
        
        if redraw && queue.is_empty() {
            core.clear_to_color(Color::from_rgb_f(0.0, 0.0, 0.0));
            draw_player(&core, &world, &sprites);
            core.flip_display();
            
            redraw = false;
        }
    }
}

fn process_physics(world: &World, space: &RigidBodySet) {
    world.lookup("player").get::<(&Handle, &mut Position, &mut Rotation, &Center)>(|(handle, pos, rot, center)| {
        let body = space.get(handle.handle).unwrap();

        pos.x = body.translation().x - center.cx;
        pos.y = body.translation().y - center.cy;
        rot.angle = body.rotation().angle();
    });
}

fn process_player(key: &HashMap<KeyCode, u32>, world: &World, space: &mut RigidBodySet) {
    world.lookup("player").get::<(&Handle, &mut Weapon, &mut Ship)>(|(handle, weapon, ship)| {
        let body = space.get_mut(handle.handle).unwrap();
    	let impulse = 30.0 * body.mass();
    	let angular_impulse = 13.0 * body.mass();

        ship.tracing = false;
    	
        if *key.get(&KeyCode::_1).unwrap() > 0 {
            weapon.kind = WeaponKind::OneBullet;
        } else if *key.get(&KeyCode::_2).unwrap() > 0 {
            weapon.kind = WeaponKind::TwoBullets;
        }

        if *key.get(&KeyCode::A).unwrap() > 0 {
            body.apply_impulse(vector![-impulse, 0.0], true);
            ship.tracing = true;
        }
        if *key.get(&KeyCode::D).unwrap() > 0 {
            body.apply_impulse(vector![impulse, 0.0], true);
            ship.tracing = true;
        }
        if *key.get(&KeyCode::Up).unwrap() > 0 {
            body.apply_impulse(vector![0.0, -impulse], true);
            ship.tracing = true;
        }
        if *key.get(&KeyCode::Down).unwrap() > 0 {
            body.apply_impulse(vector![0.0, impulse], true);
            ship.tracing = true;
        }
        if *key.get(&KeyCode::Right).unwrap() > 0 {
            body.apply_torque_impulse(angular_impulse, true);
        }
        if *key.get(&KeyCode::Left).unwrap() > 0 {
            body.apply_torque_impulse(-angular_impulse, true);
        }

        if *key.get(&KeyCode::Q).unwrap() > 0 {
            if ship.speed > 0 { ship.speed = ship.speed - 1; }
        } else if *key.get(&KeyCode::E).unwrap() > 0 {
            if ship.speed < 50 { ship.speed = ship.speed + 1; }
        } else if *key.get(&KeyCode::Minus).unwrap() > 0 {
            ship.speed = 0;
        } else if *key.get(&KeyCode::Equals).unwrap() > 0 {
            ship.speed = 50;
        }

        if *key.get(&KeyCode::Space).unwrap() > 0 {
            let linear_damping = body.linear_damping();
            let angular_damping = body.angular_damping();

            if linear_damping < 100.0 { body.set_linear_damping(linear_damping * 1.2 + 0.5); }
            if angular_damping < 100.0 { body.set_angular_damping(angular_damping * 1.2 + 0.5); }
        } else {
            body.set_linear_damping((50.0 - ship.speed as f32) / 10.0);
            body.set_angular_damping((50.0 - ship.speed as f32) / 10.0);
        }
    });
}

fn draw_player(core: &Core, world: &World, sprites: &Sprites) {
    let player = world.lookup("player");
    
    player.get::<(&Rotation, &Sprite, &Position, &Center, &mut Ship)>(|(rot, sprite, pos, center, ship)| {
        if rot.angle == 0.0 {
            core.draw_bitmap(bmp_sprite(&sprites, sprite.key).as_ref(), pos.x, pos.y, Flag::zero());
        } else {
            core.draw_rotated_bitmap(bmp_sprite(&sprites, sprite.key).as_ref(), center.cx, center.cy, pos.x + center.cx, pos.y + center.cy, rot.angle, Flag::zero());
        }

        if ship.tracing {
            let index = ((ship.speed - 1) / 5) as usize;
            let bitmap = &sprites.traces.get(ship.trace.key).unwrap()[index];

            if ship.trace.tint < 255 { ship.trace.tint += 5; }

            core.draw_tinted_rotated_bitmap(
                bitmap,
                Color::from_rgb(255 - ship.trace.tint, 255, 255),
                (bitmap.get_width() / 2) as f32,
                -center.cy,
                pos.x + center.cx,
                pos.y + center.cy,
                rot.angle,
                Flag::zero());
        } else { ship.trace.tint = 0; }
    });
}

fn init_player(world: &World, sprites: &Sprites) {
    let ship = bmp_sprite(sprites, "ship_a");
    let width = ship.get_width();
    let height = ship.get_height();

    world.entity_named("player")
        .set(Sprite { key: "ship_a" })
        .set(Size { width, height })
        .set(Position { x: (DISPLAY_CENTER_X - width / 2) as f32, y: (DISPLAY_CENTER_Y - height / 2) as f32 })
        .set(Center { cx: (width / 2) as f32, cy: (height / 2) as f32 })
        .set(Rotation { angle: 0.0 })
        .set(Weapon { kind: WeaponKind::OneBullet })
        .set(Ship { speed: 50, tracing: false, trace: Trace { key: "thin", tint: 0 }})
        .set(Handle { handle: RigidBodyHandle::invalid() });
}

fn init_physics(world: &World, mut space: &mut RigidBodySet, colliders: &mut ColliderSet) {
    world.lookup("player").get::<(&Size, &mut Handle)>(|(size, handle)| {
        let body = RigidBodyBuilder::new(RigidBodyType::Dynamic)
            .translation(vector![DISPLAY_CENTER_X as f32, DISPLAY_CENTER_Y as f32])
            .build();
        handle.handle = space.insert(body);

        let collider = ColliderBuilder::cuboid((size.width / 2) as f32, (size.height / 2) as f32)
            .density(1.0)
            .friction(0.1)
            .build();

        colliders.insert_with_parent(collider, handle.handle, &mut space);
    });
}

fn bmp_sprite(sprites: &Sprites, key: &str) -> Rc<SubBitmap> {
    sprites.sprites.get(key).unwrap().upgrade().unwrap()
}

fn init_sprites(core: &Core) -> Sprites {
    let sheet = Bitmap::load(core, "assets/spritesheet.png").unwrap();
    let sheet_px = Bitmap::load(core, "assets/spritesheet-px.png").unwrap();

    let sprites: HashMap<&str, Weak<SubBitmap>> = HashMap::from([
        ("effect_purple", sheet.create_sub_bitmap(156, 32, 32, 64).unwrap()),
        ("effect_yellow", sheet.create_sub_bitmap(180, 181, 32, 64).unwrap()),
        ("enemy_a", sheet.create_sub_bitmap(0, 420, 48, 48).unwrap()),
        ("enemy_b", sheet.create_sub_bitmap(100, 140, 48, 48).unwrap()),
    	("enemy_c", sheet.create_sub_bitmap(0, 0, 64, 32).unwrap()),
    	("enemy_d", sheet.create_sub_bitmap(100, 188, 48, 48).unwrap()),
    	("enemy_e", sheet.create_sub_bitmap(100, 332, 48, 48).unwrap()),
    	("meteor_detailed_large", sheet.create_sub_bitmap(144, 428, 48, 48).unwrap()),
    	("meteor_detailed_small", sheet.create_sub_bitmap(144, 476, 32, 32).unwrap()),
    	("meteor_large", sheet.create_sub_bitmap(144, 380, 48, 48).unwrap()),
    	("meteor_small", sheet.create_sub_bitmap(112, 0, 32, 32).unwrap()),
    	("meteor_square_detailed_large", sheet.create_sub_bitmap(108, 32, 48, 48).unwrap()),
    	("meteor_square_detailed_small", sheet.create_sub_bitmap(152, 96, 32, 32).unwrap()),
    	("meteor_square_large", sheet.create_sub_bitmap(52, 340, 48, 48).unwrap()),
    	("meteor_square_small", sheet.create_sub_bitmap(144, 0, 32, 32).unwrap()),
    	("satellite_a", sheet.create_sub_bitmap(0, 148, 52, 44).unwrap()),
    	("satellite_b", sheet.create_sub_bitmap(0, 192, 52, 52).unwrap()),
    	("satellite_c", sheet.create_sub_bitmap(0, 332, 52, 36).unwrap()),
    	("satellite_d", sheet.create_sub_bitmap(0, 296, 52, 36).unwrap()),
    	("ship_a", sheet.create_sub_bitmap(52, 388, 32, 24).unwrap()),
    	("ship_b", sheet.create_sub_bitmap(148, 341, 16, 32).unwrap()),
    	("ship_c", sheet.create_sub_bitmap(48, 468, 48, 32).unwrap()),
    	("ship_d", sheet.create_sub_bitmap(64, 0, 48, 32).unwrap()),
    	("ship_e", sheet.create_sub_bitmap(96, 436, 48, 48).unwrap()),
    	("ship_f", sheet.create_sub_bitmap(96, 388, 48, 48).unwrap()),
    	("ship_g", sheet.create_sub_bitmap(60, 32, 48, 48).unwrap()),
    	("ship_h", sheet.create_sub_bitmap(56, 92, 48, 48).unwrap()),
    	("ship_i", sheet.create_sub_bitmap(148, 261, 32, 48).unwrap()),
    	("ship_j", sheet.create_sub_bitmap(52, 292, 48, 48).unwrap()),
    	("ship_k", sheet.create_sub_bitmap(148, 181, 32, 48).unwrap()),
    	("ship_l", sheet.create_sub_bitmap(52, 244, 48, 48).unwrap()),
    	("ship_sides_a", sheet.create_sub_bitmap(148, 128, 42, 53).unwrap()),
    	("ship_sides_b", sheet.create_sub_bitmap(0, 244, 52, 52).unwrap()),
    	("ship_sides_c", sheet.create_sub_bitmap(0, 368, 52, 52).unwrap()),
    	("ship_sides_d", sheet.create_sub_bitmap(0, 468, 48, 32).unwrap()),
    	("star_large", sheet.create_sub_bitmap(52, 196, 48, 48).unwrap()),
    	("star_medium", sheet.create_sub_bitmap(52, 148, 48, 48).unwrap()),
    	("star_small", sheet.create_sub_bitmap(96, 484, 16, 16).unwrap()),
    	("star_tiny", sheet.create_sub_bitmap(112, 484, 16, 16).unwrap()),
    	("station_a", sheet.create_sub_bitmap(48, 420, 48, 48).unwrap()),
    	("station_b", sheet.create_sub_bitmap(0, 92, 56, 56).unwrap()),
    	("station_c", sheet.create_sub_bitmap(0, 32, 60, 60).unwrap()),
        ("bullet_a", sheet_px.create_sub_bitmap(13, 0, 2, 9).unwrap())]);

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

	let traces: HashMap<&str, Vec<Bitmap>> = HashMap::from([
	    ("thin", trace_thin), ("medium", trace_medium), ("thick", trace_thick)]);
	
	let spark: [Weak<SubBitmap>; 3] = [
	    sheet_px.create_sub_bitmap(34, 0, 10, 8).unwrap(),
	    sheet_px.create_sub_bitmap(45, 0, 7, 8).unwrap(),
        sheet_px.create_sub_bitmap(54, 0, 9, 8).unwrap()];

    Sprites { sheet, sheet_px, sprites, traces, spark }
}
