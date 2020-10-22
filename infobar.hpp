#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>

#include "defs.hpp"

struct InfoBar {
	InfoBar(SDL_Renderer *renderer, SDL_Window *window, TTF_Font *font, WindowDim *dim);
	~InfoBar();

	void set_has_edited(bool edited);

	void render();
	void update_texture();
public:
	std::string text;
	int cursor_y;

	bool has_edited = false;

	WindowDim *window_dim;
	
	TTF_Font *font;
	SDL_Renderer *renderer;
	SDL_Window *window;

	SDL_Surface *surface;
	SDL_Texture *texture;

	int char_width, char_height;
};
