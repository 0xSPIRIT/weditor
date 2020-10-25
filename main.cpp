#define SDL_MAIN_HANDLED

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "buffer.hpp"
#include "infobar.hpp"

#include "defs.hpp"

Editor_Colors editor_colors;

int strlen(const char *str) {
	size_t len = 0;
	while (str[len++]) {}
	return len;
}

int main(int argc, char **argv) {
	std::string start_text;
	if (argc > 2) {
		fprintf(stderr, "Usage: %s <file> OR just %s.", argv[0]);
		return 1;
	} else if (argc == 2) {
		start_text = argv[1];
	}
	
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	IMG_Init(IMG_INIT_PNG);

	WindowDim window_dim;
	SDL_Window *window = SDL_CreateWindow("*buffer* - weditor",
										  SDL_WINDOWPOS_UNDEFINED,
										  SDL_WINDOWPOS_UNDEFINED,
										  window_dim.width,
										  window_dim.height,
										  SDL_WINDOW_RESIZABLE);

	SDL_Renderer *renderer = SDL_CreateRenderer(window,
												-1,
												0);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	
	bool running = true;

	SDL_Surface *window_icon_surf = IMG_Load("weditor_icon.png");
	SDL_SetWindowIcon(window, window_icon_surf);

	SDL_Color col = { 255, 255, 255 };
	TTF_Font *font = TTF_OpenFont("fonts/8514oem.fon", 18);

	InfoBar bar(renderer, window, font, &window_dim);
	bar.text = "*buffer*";

	Buffer buffer(renderer, window, font, &bar, &window_dim, &start_text[0], false);
	if (argc == 2) {
		buffer.load_from_file(argv[1]);
		bar.text = argv[1];
	}
	bar.update_texture();
	
	Buffer mini_buffer(renderer, window, font, &bar, &window_dim, "", true);
	mini_buffer.main_buffer = &buffer;
	buffer.mini_buffer = &mini_buffer;
	
	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) running = false;
			if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					window_dim.width = event.window.data1;
					window_dim.height = event.window.data2;
				}
			}
			
			buffer.event_update(event);
			mini_buffer.event_update(event);

			SDL_SetRenderDrawColor(renderer,
								   editor_colors.bg.r,
								   editor_colors.bg.g,
								   editor_colors.bg.b,
								   editor_colors.bg.a);
			SDL_RenderClear(renderer);

			buffer.render_highlight_line();
			buffer.render_mark();
			buffer.render();
		
			bar.render();
		
			mini_buffer.render();
			buffer.render_cursor();
			mini_buffer.render_cursor();

			SDL_RenderPresent(renderer);
		}
	}

	SDL_FreeSurface(window_icon_surf);
	TTF_CloseFont(font);
	TTF_Quit();
	IMG_Quit();
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
}
