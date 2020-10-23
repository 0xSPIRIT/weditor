#define SDL_MAIN_HANDLED

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "buffer.hpp"
#include "infobar.hpp"

#include "defs.hpp"

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
	bool running = true;

	SDL_Color col = { 255, 255, 255 };
	TTF_Font *font = TTF_OpenFont("fonts/lucon.ttf", 18);

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

			SDL_SetRenderDrawColor(renderer, 6, 35, 41, 255);
			SDL_RenderClear(renderer);

			buffer.render_mark();
			buffer.render();
		
			bar.render();
		
			mini_buffer.render();
			buffer.render_cursor();
			mini_buffer.render_cursor();

			SDL_RenderPresent(renderer);
		}
	}

	TTF_CloseFont(font);
	TTF_Quit();
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
}
