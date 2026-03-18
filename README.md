This is a simple space game.\
I am making it in my spare time for fun and learning.\
Feel free to use it as template for your own things.\
\
![Screenshot](pics/screen.png)\
\
Space worlds (game maps) with entities for this game you can create and edit with [moeditor](https://github.com/moscout/moeditor) project. There you will find also description of the game storage (SQLite database) structure.\
\
You don't have to compile the game to run it, all the necessary release files (for windows) are located in the folder **bin**.\
\
I am using [MSYS2](https://www.msys2.org/) for managing libraries that I use for development in C.

### Dependencies
| Tool                                      | Purpose                                                           |
|-------------------------------------------|-------------------------------------------------------------------|
| [box2d](https://packages.msys2.org/base/mingw-w64-box2d) | For game physics.                                  |
| [raylib](https://packages.msys2.org/base/mingw-w64-raylib) | For graphics and user events.                    |
| [raygui](https://github.com/raysan5/raygui) | For immediate-mode gui.                                         |
| [flecs](https://packages.msys2.org/base/mingw-w64-flecs) | Fast entity component system.                      |
| [sqlite](https://packages.msys2.org/base/mingw-w64-sqlite3) | For reading/writing to database.                |
| [hashmap](https://github.com/tidwall/hashmap.c.git) | Hash map implementation in C.                           |
| [kenney](http://kenney.nl) | For assets.                                                                      |

### Implemented features
- Moving forward, backward, left, right, angular.
- Extreme (fast) breaking (to full stop).
- Increasing/decreasing max speed.
- Choose weapon type (one/two bullets for now).
- Shots interval (100 ms).
- Impulses (for moving) calculates depending on ship mass.
- Bullet/asteroid contact (collision) animation.

### Control keys
| Key      | Description                             |
|----------|-----------------------------------------|
| 1        | Change weapon to one bullet.            |
| 2        | Change weapon to two bullets.           |
| A        | Move left.                              |
| D        | Move right.                             |
| Up       | Move forward.                           |
| Down     | Move backward.                          |
| Left     | Turn left.                              |
| Right    | Turn right.                             |
| Minus    | Minimize speed.                         |
| Plus(=)  | Maximize speed.                         |
| Q        | Decrease speed (slowly).                |
| E        | Increase speed (slowly).                |
| Space    | Brake (extreme).                        |
| W        | Shoot.                                  |
| Ctrl+I   | Zoom in.                                |
| Ctrl+O   | Zoom out.                               |
| Ctrl+F   | Full-screen on/off.                     |
| Esc      | Show menu (game pause) or back to game. |

### Related projects
| Project                                             | Description                                             |
|-----------------------------------------------------|---------------------------------------------------------|
| [moeditor](https://github.com/moscout/moeditor) | Worlds editor for space game.                           |
