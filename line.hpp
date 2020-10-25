#pragma once

#include <string>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "defs.hpp"

struct Line {
	Line(SDL_Renderer *renderer, TTF_Font *font);
	~Line();

	void add_chars(size_t index, const char *chars);
	void remove_char(size_t index);
	
	void render(int xoff, int yoff);
	void update_texture();
public:
	std::string text, prefix;
	size_t length = 0;

	TTF_Font *font = NULL;
	SDL_Color color = editor_colors.fg;
	SDL_Color minibuffer_color = editor_colors.fg;

	SDL_Renderer *renderer;

	SDL_Surface *surface = NULL;
	SDL_Texture *texture = NULL;
};
