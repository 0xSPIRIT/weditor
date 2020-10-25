#pragma once

struct WindowDim {
	int width = 900, height = 800;
};

struct Editor_Colors {
	SDL_Color bg = { 18, 31, 46, 255 };
	SDL_Color fg = { 188, 210, 238, 255 };
	SDL_Color bar = { 21, 44, 70, 255 };
};

extern Editor_Colors editor_colors;
