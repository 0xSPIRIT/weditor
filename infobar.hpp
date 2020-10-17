#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>

struct InfoBar {
	InfoBar(SDL_Renderer *renderer, TTF_Font *font);
	~InfoBar();

	void render();
	void update_texture();
public:
	std::string text;
	int cursor_y;
	
	TTF_Font *font;
	SDL_Renderer *renderer;

	SDL_Surface *surface;
	SDL_Texture *texture;

	int char_width, char_height;
};
