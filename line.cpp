#include "line.hpp"

Line::Line(SDL_Renderer *renderer, TTF_Font *font) {
	this->renderer = renderer;
	this->font = font;
}

Line::~Line() {
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
}

void Line::add_chars(size_t index, const char *chars) {
	text.insert(index, std::string(chars));
	update_texture();
}

void Line::remove_char(size_t index) {
	text.erase(text.begin() + index - 1);
	update_texture();
}

void Line::render(int yoff) {
	if (!texture) return;
	
	SDL_Rect rect = { 0, yoff, surface->w, surface->h };

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderCopy(renderer, texture, NULL, &rect);
}

void Line::update_texture() {
	std::string str = prefix + text;
	
	surface = TTF_RenderText_Solid(font,
								   &str[0],
								   color);
	texture = SDL_CreateTextureFromSurface(renderer, surface);
}
