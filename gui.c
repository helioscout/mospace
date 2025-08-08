#pragma once

#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "style_cyber.h"

#include "components.c"

#if !defined(RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT)
	#define RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT 24
#endif

#if !defined(RAYGUI_WINDOW_CLOSEBUTTON_SIZE)
	#define RAYGUI_WINDOW_CLOSEBUTTON_SIZE 18
#endif

void gui_window(GameState *state, GameGui *gui, window_t *win, void (*draw_content)(GameState*, GameGui*, Vector2, Vector2),
    Vector2 content_size, const char* title) {

	int close_title_size_delta_half = (RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT - RAYGUI_WINDOW_CLOSEBUTTON_SIZE) / 2;
	Vector2 *position = &win->position;
	Vector2 *size     = &win->size;
	Vector2 *scroll   = &win->scroll;
	bool *minimized   = &win->minimized;
	bool *moving      = &win->moving;
	bool *resizing    = &win->resizing;

    // Window movement and resize input and collision check.
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !*moving && !*resizing) {
        Vector2 mouse_position = GetMousePosition();

        Rectangle title_collision_rect = {
            .x = position->x,
            .y = position->y,
            .width = size->x - (RAYGUI_WINDOW_CLOSEBUTTON_SIZE + close_title_size_delta_half),
            .height = RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT };
        
        Rectangle resize_collision_rect = {
            .x = position->x + size->x - 20,
            .y = position->y + size->y - 20,
            .width = 20,
            .height = 20 };

        if (CheckCollisionPointRec(mouse_position, title_collision_rect)) *moving = true;
        else if(!*minimized && CheckCollisionPointRec(mouse_position, resize_collision_rect)) *resizing = true;
    }

    // Window movement and resize update.
    if (*moving) {
        Vector2 mouse_delta = GetMouseDelta();
        position->x += mouse_delta.x;
        position->y += mouse_delta.y;

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            *moving = false;

            // Clamp window position keep it inside the application area.
            if (position->x < 0) position->x = 0;
            else if (position->x > GetScreenWidth() - size->x) position->x = GetScreenWidth() - size->x;
            if (position->y < 0) position->y = 0;
            else if(position->y > GetScreenHeight()) position->y = GetScreenHeight() - RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT;
        }
    } else if (*resizing) {
        Vector2 mouse = GetMousePosition();

        if (mouse.x > position->x) size->x = mouse.x - position->x;
        if (mouse.y > position->y) size->y = mouse.y - position->y;

        // Clamp window size to an arbitrary minimum value and the window size as the maximum.
        if (size->x < 100) size->x = 100;
        else if (size->x > GetScreenWidth()) size->x = GetScreenWidth();
        if (size->y < 100) size->y = 100;
        else if (size->y > GetScreenHeight()) size->y = GetScreenHeight();

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) *resizing = false;
    }

    // Window and content drawing with scissor and scroll area.
    if (*minimized) {
        GuiStatusBar((Rectangle) {
            .x = position->x,
            .y = position->y,
            .width = size->x,
            .height = RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT
        }, title);

        if (GuiButton((Rectangle) {
            .x = position->x + size->x - RAYGUI_WINDOW_CLOSEBUTTON_SIZE - close_title_size_delta_half,
            .y = position->y + close_title_size_delta_half,
            .width = RAYGUI_WINDOW_CLOSEBUTTON_SIZE,
            .height = RAYGUI_WINDOW_CLOSEBUTTON_SIZE
        }, "#120#")) {
            *minimized = false;
        }
    } else {
        *minimized = GuiWindowBox((Rectangle) {
            .x = position->x,
            .y = position->y,
            .width = size->x,
            .height = size->y
        }, title);

        // Scissor and draw content within a scroll panel.
        if (draw_content != NULL) {
            Rectangle scissor = { 0 };
            
            GuiScrollPanel((Rectangle) {
                .x = position->x,
                .y = position->y + RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT,
                .width = size->x,
                .height = size->y - RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT
            }, NULL, (Rectangle) {
                .x = position->x,
                .y = position->y,
                .width = content_size.x,
                .height = content_size.y
            }, scroll, &scissor);

            bool require_scissor = size->x < content_size.x || size->y < content_size.y;

            if (require_scissor) BeginScissorMode(scissor.x, scissor.y, scissor.width, scissor.height);

            draw_content(state, gui, *position, *scroll);

            if (require_scissor) EndScissorMode();
        }

        // Draw the resize button/icon.
        GuiDrawIcon(71, position->x + size->x - 20, position->y + size->y - 20, 1, WHITE);
    }
}

void gui_menu(GameState *state, GameGui *gui, Vector2 pos, Vector2 scroll) {
    if (GuiButton((Rectangle) {
        .x = pos.x + 10 + scroll.x,
        .y = pos.y + RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT + 40 + scroll.y,
        .width = 270,
        .height = 20 }, "LOAD WORLD")) {
        /* We need prevent drop down selection clicks handling. */
        if (!gui->gui_menu->worlds_editing) state->actions |= LoadWorld;
    }

    if (GuiButton((Rectangle) {
        .x = pos.x + 10 + scroll.x,
        .y = pos.y + RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT + 70 + scroll.y,
        .width = 270,
        .height = 20 }, "EXIT GAME")) {
        /* We need prevent drop down selection clicks handling. */
        if (!gui->gui_menu->worlds_editing) state->actions |= ExitGame;
    }

    if (GuiDropdownBox((Rectangle) {
        .x = pos.x + 10 + scroll.x,
        .y = pos.y + RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT + 10 + scroll.y,
        .width = 270,
        .height = 20 },
        gui->gui_menu->worlds_count
            ? TextJoin((const char**)gui->gui_menu->world_labels, gui->gui_menu->worlds_count, ";")
            : "",
        &gui->gui_menu->world_index, gui->gui_menu->worlds_editing)) {
        gui->gui_menu->worlds_editing = !gui->gui_menu->worlds_editing;
    }
}

void gui_draw(GameState *state, GameGui *gui) {
	if (state->screen == Menu) {
		gui_window(state, gui, gui->w_menu, gui_menu, (Vector2) { .x = 270, .y = 110 },
		    GuiIconText(ICON_GEAR_BIG, "MENU"));
	}
}
