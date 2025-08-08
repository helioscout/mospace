#pragma once

#include "components.c"

void gui_layout(GameState *state, GameGui *gui) {
	gui->w_menu->position.x = state->size.x / 2 - gui->w_menu->size.x / 2;
	gui->w_menu->position.y = state->size.y / 2 - gui->w_menu->size.y / 2;
}
