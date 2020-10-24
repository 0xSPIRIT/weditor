#pragma once

#include "line.hpp"
#include "infobar.hpp"

#include "defs.hpp"

#include <vector>

enum MiniBufferMode {
					 MB_SaveFile,
					 MB_LoadFile,
					 MB_GotoLine,
					 MB_SetCompileCommand,
					 MB_SetProgramExecutable
};

struct Buffer {
	Buffer(SDL_Renderer *renderer, SDL_Window *window,
		   TTF_Font *font, InfoBar *bar, WindowDim *dim,
		   const char *start, bool is_minibuf);
	~Buffer();

	void type(const char *text);
	void minibuffer_clear();

	void cursor_move_up();
	void cursor_move_down();
	void cursor_move_left();
	void cursor_move_right();

	void cursor_forward_word();
	void cursor_backward_word();

	void kill_line(const Uint8 *keyboard);

	void delete_previous_word();
	void delete_next_word();
	
	void backspace(const Uint8 *keyboard);

	void set_cursor_y(int y);
	void set_cursor_x(int x);

	void update_view();
	void center_view();

	void view_down();
	void view_up();

	void toggle_overwrite_mode();

	bool is_meta_pressed(const Uint8 *keyboard);
	bool is_ctrl_pressed(const Uint8 *keyboard);
	
	bool is_line_empty();
	bool is_char_separator();
	
	void clamp_cursor();
	void render_cursor();

	void mark_start();
	void mark_end();
	void kill_mark();

	void update_mark();
	void render_mark();

	void render_highlight_line();

	bool load_from_file(const char *file);
	
	void render();
	void event_update(const SDL_Event &event);
public:
	SDL_Renderer *renderer;
	SDL_Window *window;

	bool is_mark_open = false;
	int mark_start_x, mark_start_y;
	int mark_end_x, mark_end_y;

	std::string compile_command, program_executable;

	WindowDim *window_dim;
	
	TTF_Font *font;
	Buffer *mini_buffer = nullptr;

	int view_x = 0;
	int view_y = 0;

	int scroll_by = 5; // chars

	InfoBar *infobar;

	bool in_focus = true;

	int cursor_x = 0, cursor_y = 0;
	int char_width, char_height;

	bool overwrite_mode = false;
	std::vector<Line *> lines;

	std::vector<char> word_separators =
		{
			' ', '(', ')', '[', ']', '_',
			'<', '>', ',', '.', '=', ';',
			'#', '\'', '\"', ':', '{', '}',
			'\\' , '+', '-', '*', '/'
		};
	
	// MiniBuffer:
	bool is_minibuffer;
	MiniBufferMode mode;
	Buffer *main_buffer = nullptr;
};
