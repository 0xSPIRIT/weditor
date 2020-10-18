#pragma once

#include "line.hpp"
#include "infobar.hpp"

#include "defs.hpp"

#include <vector>

enum MiniBufferMode {
					 MB_SaveFile,
					 MB_LoadFile,
					 MB_GotoLine
};

struct Buffer {
	Buffer(SDL_Renderer *renderer, SDL_Window *window,
		   TTF_Font *font, InfoBar *bar, WindowDim *dim,
		   const char *start, bool is_minibuf);
	~Buffer();

	void cursor_move_up();
	void cursor_move_down();
	void cursor_move_left();
	void cursor_move_right();

	void cursor_forward_word();
	void cursor_backward_word();

	void set_cursor_y(int y);
	void set_cursor_x(int x);

	void update_view();
	void center_view();

	bool is_line_empty();
	
	void clamp_cursor();
	void render_cursor();

	bool load_from_file(const char *file);
	
	void render();
	void event_update(const SDL_Event &event);
public:
	SDL_Renderer *renderer;
	SDL_Window *window;

	WindowDim *window_dim;
	
	TTF_Font *font;
	Buffer *mini_buffer = nullptr;

	int view_y = 0;

	InfoBar *infobar;

	bool in_focus = true;

	int cursor_x = 0, cursor_y = 0;
	int char_width, char_height;
	
	std::vector<Line *> lines;

	std::vector<char> word_separators = { ' ', '(', ')', '[', ']', '_' };
	
	// MiniBuffer:
	bool is_minibuffer;
	MiniBufferMode mode;
	Buffer *main_buffer = nullptr;
};
