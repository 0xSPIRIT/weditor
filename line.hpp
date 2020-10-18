#pragma once

#include <string>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

struct Line {
	Line(SDL_Renderer *renderer, TTF_Font *font);
	~Line();

	void add_chars(size_t index, const char *chars);
	void remove_char(size_t index);
	
	void render(int yoff);
	void update_texture();
public:
	std::string text, prefix;
	size_t length = 0;

	TTF_Font *font = NULL;
	SDL_Color color = { 209, 184, 151 };
	SDL_Color minibuffer_color = { 209, 184, 51 };

	SDL_Renderer *renderer;

	SDL_Surface *surface = NULL;
	SDL_Texture *texture = NULL;
};
