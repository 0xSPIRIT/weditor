#include "infobar.hpp"

#include "defs.hpp"

InfoBar::InfoBar(SDL_Renderer *renderer, TTF_Font *font, WindowDim *dim) {
	this->window_dim = dim;
	this->renderer = renderer;
	this->font = font;

	TTF_SizeText(font, "-", &char_width, &char_height);
}

InfoBar::~InfoBar() {
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
}

void InfoBar::update_texture() {
	SDL_Color color = { 6, 35, 41 };

	std::string s = text + (has_edited ? "*" : " ") +
		"    L" + std::to_string(cursor_y);
	
	surface = TTF_RenderText_Blended(font,
 									 &s[0],
									 color);

	texture = SDL_CreateTextureFromSurface(renderer, surface);
}

void InfoBar::render() {
	SDL_Rect rect = { 0, window_dim->height - char_height * 2, window_dim->width, char_height };
	SDL_SetRenderDrawColor(renderer, 209, 181, 151, 255);
	SDL_RenderFillRect(renderer, &rect);

	SDL_Rect textrect = { char_width, window_dim->height - char_height * 2, surface->w, surface->h };
	
	SDL_RenderCopy(renderer, texture, NULL, &textrect);
}
