#include "infobar.hpp"

#include "defs.hpp"

InfoBar::InfoBar(SDL_Renderer *renderer, SDL_Window *window, TTF_Font *font, WindowDim *dim) {
	this->window_dim = dim;
	this->renderer = renderer;
	this->window = window;
	this->font = font;

	TTF_SizeText(font, "-", &char_width, &char_height);
}

InfoBar::~InfoBar() {
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
}

void InfoBar::set_has_edited(bool edited) {
	has_edited = edited;

	
	std::string str = text;
	if (edited) {
		str += "*";
	}
	str += " - weditor";
	SDL_SetWindowTitle(window, &str[0]);
}

void InfoBar::update_texture() {
	std::string s = text;
	if (has_edited) {
		s += "*";
	}
	s += "    L" + std::to_string(cursor_y);
	
	surface = TTF_RenderText_Blended(font,
									 &s[0],
									 editor_colors.fg);

	texture = SDL_CreateTextureFromSurface(renderer, surface);
}

void InfoBar::render() {
	SDL_Rect rect = { 0, window_dim->height - char_height * 2, window_dim->width, char_height };
	SDL_SetRenderDrawColor(renderer,
						   editor_colors.bar.r,
						   editor_colors.bar.g,
						   editor_colors.bar.b,
						   editor_colors.bar.a);
	SDL_RenderFillRect(renderer, &rect);

	SDL_Rect textrect = { char_width, window_dim->height - char_height * 2, surface->w, surface->h };
	
	SDL_RenderCopy(renderer, texture, NULL, &textrect);
}
