#define SDL_MAIN_HANDLED

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <windows.h>

#include "buffer.hpp"
#include "infobar.hpp"

#include "defs.hpp"

int main(int argc, char **argv) {
	if (argc > 2) {
		fprintf(stderr, "Usage: %s <file> OR just %s.", argv[0]);
		return 1;
	}
	
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();

	std::string title = "weditor@";
	TCHAR computer_name[128];
	DWORD buf_count = 128;
	GetComputerName(computer_name, &buf_count);
	title += computer_name;

	SDL_Window *window = SDL_CreateWindow(&title[0],
										  SDL_WINDOWPOS_UNDEFINED,
										  SDL_WINDOWPOS_UNDEFINED,
										  WINDOW_WIDTH,
										  WINDOW_HEIGHT,
										  0);
	
	SDL_Renderer *renderer = SDL_CreateRenderer(window,
												-1,
												0);
	bool running = true;

	SDL_Color col = { 255, 255, 255 };
	TTF_Font *font = TTF_OpenFont("fonts/LiberationMono-Regular.ttf", 16);

	InfoBar bar(renderer, font);
	bar.text = "*buffer*";

	Buffer buffer(renderer, font, &bar, "", false);
	if (argc == 2) {
		buffer.load_from_file(argv[1]);
		bar.text = argv[1];
	}
	bar.update_texture();

	Buffer mini_buffer(renderer, font, &bar, "", true);
	
	mini_buffer.main_buffer = &buffer;
	buffer.mini_buffer = &mini_buffer;
	
	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) running = false;
			
			buffer.event_update(event);
			mini_buffer.event_update(event);
		}

		SDL_SetRenderDrawColor(renderer, 6, 35, 41, 255);
		SDL_RenderClear(renderer);

		buffer.render();
		
		bar.render();
		
		mini_buffer.render();
		buffer.render_cursor();
		mini_buffer.render_cursor();

		SDL_RenderPresent(renderer);
	}

	TTF_CloseFont(font);
	TTF_Quit();
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
}
