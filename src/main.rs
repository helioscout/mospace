use allegro::*;
use allegro_image::*;

const DISPLAY_WIDTH: i32 = 800;
const DISPLAY_HEIGHT: i32 = 600;

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
}
