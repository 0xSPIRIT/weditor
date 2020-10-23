#include "buffer.hpp"

#include <cstring>
#include <fstream>
#include <sstream>

#include "defs.hpp"
#include "utils.hpp"

Buffer::Buffer(SDL_Renderer *renderer, SDL_Window *window,
			   TTF_Font *font, InfoBar *bar, WindowDim *dim,
			   const char *start, bool is_minibuf) {
	this->renderer = renderer;
	this->window = window;
	this->font = font;
	this->infobar = bar;
	this->window_dim = dim;
	this->is_minibuffer = is_minibuf;

	infobar->cursor_y = 0;

	TTF_SizeText(font, "-", &char_width, &char_height);

	lines.push_back(new Line(renderer, font));
	lines[0]->text += start;
	lines[0]->update_texture();

	if (is_minibuf) {
		in_focus = false;
	}
}

Buffer::~Buffer() {
	for (auto *l : lines) {
		delete l;
	}
}

bool Buffer::load_from_file(const char *fp) {
	std::ifstream file(fp);
	if (!file.is_open()) {
		return 1;
	}

	for (auto *e : lines) delete e;
	lines.clear();

	std::string line;
	while (std::getline(file, line)) {
		replace_string_in_place(line,
								"\t",
								"    ");
		
		lines.push_back(new Line(renderer, font));
		lines.back()->text = line;
		lines.back()->update_texture();
	}
   	
	infobar->text = fp;
	infobar->update_texture();

	std::string title = infobar->text + " - weditor";
	SDL_SetWindowTitle(window, &title[0]);

	view_y = 0;
	cursor_y = 0;
	set_cursor_x(0);

	return 0;
}

void Buffer::toggle_overwrite_mode() {
	overwrite_mode = !overwrite_mode;
}

void Buffer::cursor_move_up() {
	cursor_y--;
	infobar->cursor_y = cursor_y;
	infobar->update_texture();

	if (cursor_y * char_height - view_y < 0) {
		view_y -= char_height * (int) (window_dim->height / 2 / char_height);
	}
	if (view_y < 0) view_y = 0;
	update_mark();
}

void Buffer::cursor_move_down() {
	cursor_y++;
	infobar->cursor_y = cursor_y;
	infobar->update_texture();

	if (cursor_y * char_height + char_height * 3 - view_y > window_dim->height) {
		view_y += char_height * (int) (window_dim->height / 2 / char_height);
	}

	update_mark();
}

void Buffer::cursor_move_left() {
	cursor_x--;
	update_mark();
}

void Buffer::cursor_move_right() {
	cursor_x++;
	update_mark();
}

void Buffer::cursor_forward_word() {
	while (is_char_separator()) {
		cursor_x++;
		if (cursor_x > lines[cursor_y]->text.size() && cursor_y == lines.size() - 1) {
			return;
		}
	}
	
	while (!is_char_separator()) {
		cursor_x++;
		if (cursor_x > lines[cursor_y]->text.size() && cursor_y == lines.size() - 1) {
			return;
		}
		if (cursor_x > lines[cursor_y]->text.size()) {
			cursor_move_down();
			set_cursor_x(0);
		}
	}
}

void Buffer::cursor_backward_word() {
	cursor_x--;
	if (cursor_x < 0) {
		if (cursor_y == 0) return;
		cursor_move_up();
		set_cursor_x(lines[cursor_y]->text.size());
	}
	
	while (is_char_separator()) {
		cursor_x--;
		if (cursor_y == 0 && cursor_x < 0) return;
		if (cursor_x < 0) {
			cursor_move_up();
			set_cursor_x(lines[cursor_y]->text.size());
		}
	}
	while (!is_char_separator()) {
		if (cursor_y == 0 && cursor_x < 0) return;
		cursor_x--;
		if (cursor_x < 0) {
			cursor_move_up();
			set_cursor_x(lines[cursor_y]->text.size());
		}
	}
	
	cursor_x++;
	update_mark();
}

void Buffer::delete_previous_word() {
	while (!is_char_separator() && cursor_x > 0) {
		lines[cursor_y]->remove_char(cursor_x--);
		printf("%c", lines[cursor_y]->text[cursor_x]);
	}
	lines[cursor_y]->update_texture();
	update_mark();
}

bool Buffer::is_char_separator() {
	for (char s : word_separators) {
		if ((cursor_x == 0 && !is_line_empty()) || lines[cursor_y]->text[cursor_x] == s)
			return true;
	}
	return false;
}

void Buffer::mark_start() {
	mark_start_x = cursor_x;
	mark_start_y = cursor_y;
	is_mark_open = true;
}

void Buffer::mark_end() {
	is_mark_open = false;
}

void Buffer::update_mark() {
	if (!is_mark_open) return;
	
	mark_end_x = cursor_x;
	mark_end_y = cursor_y;
}

void Buffer::kill_mark() {
	if (!is_mark_open) return;

	if (mark_start_y == mark_end_y) {
		puts("start is equal to end");
		int s = sign(mark_end_x - mark_start_x);
		if (s > 0) {
			lines[mark_start_y]->
				text.erase(lines[mark_start_y]->text.begin() + mark_start_x,
						   lines[mark_start_y]->text.begin() + mark_end_x);
			set_cursor_x(cursor_x + (mark_start_x - mark_end_x));
		} else if (s < 0) {
			lines[mark_start_y]->
				text.erase(lines[mark_start_y]->text.begin() + mark_end_x,
						   lines[mark_start_y]->text.begin() + mark_start_x);
		}
		lines[mark_start_y]->update_texture();
	} else if (mark_start_y < mark_end_y) {
		puts("start is less than end");
		printf("Mark start x: %d, mark end x: %d\n", mark_start_x, mark_end_x);

		int count = mark_end_y - mark_start_y;
		for (int i = 0; i < count; ++i) {
			
			int sx, sy, ex;

			sy = (mark_start_y + i);
			ex = lines[mark_start_y + i]->text.size();
			if (i == 0) {
				sx = mark_start_x * char_width - view_x;
				if (i < count-1 && sx == 0) {
					lines.erase(lines.begin() + mark_start_y + i);
					continue;
				}
			} else if (i == count - 1) {
				sx = 0;
				ex = mark_end_x;
				if (i < count-1 && sx == 0) {
					lines.erase(lines.begin() + mark_start_y + i);
					continue;
				}
			} else {
				sx = 0;
				lines.erase(lines.begin() + mark_start_y + i);
				continue;
			}

			lines[mark_start_y + i]->text.erase
				(lines[mark_start_y + i]->text.begin() + mark_start_x,
				 lines[mark_start_y + i]->text.begin() + mark_end_x);
			lines[mark_start_y + i]->update_texture();
		}
	}

	mark_end();

	if (!is_minibuffer) {
		infobar->set_has_edited(true);
		infobar->update_texture();
	}
}

void Buffer::render_mark() {
	if (!is_mark_open) return;
	
	if (mark_start_y == mark_end_y) {
		SDL_Rect rect = {
			mark_start_x * char_width - view_x,
			mark_start_y * char_height - view_y,
			(mark_end_x - mark_start_x) * char_width,
			char_height
		};
		
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		SDL_RenderFillRect(renderer, &rect);
	} else if (mark_start_y < mark_end_y) {
		int count = mark_end_y - mark_start_y + 1;
		for (int i = 0; i < count; ++i) {
			SDL_Rect rect;
			int sx, sy, ex;
			
			sy = (mark_start_y + i) * char_height - view_y;
			ex = window_dim->width + view_x;
			if (i == 0) {
				sx = mark_start_x * char_width - view_x;
			} else if (i == count - 1) {
				sx = 0; // '- view_y' but because its always less than 0 we don't bother.
				ex = mark_end_x * char_width - view_x;
			} else {
				sx = 0; 
			}
			
			rect = {
				sx,
				sy,
				ex,
				char_height
			};
			SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
			SDL_RenderFillRect(renderer, &rect);
			
		}
	} else {
		int count = mark_end_y - mark_start_y - 1;
		for (int i = 0; i > count; i--) {
			SDL_Rect rect;
			int sx, sy, ex;
				
			sy = (mark_start_y + i) * char_height - view_y;
			ex = window_dim->width + view_x;
			if (i == 0) {
				ex = mark_start_x * char_width - view_x;
				sx = 0;
			} else if (i == count + 1){
				ex = window_dim->width;
				sx = mark_end_x * char_width - view_x;
			} else {
				sx = 0; 
			}
				
			rect = {
				sx,
				sy,
				ex,
				char_height
			};
			SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
			SDL_RenderFillRect(renderer, &rect);
		}
	}
}


void Buffer::set_cursor_y(int y) {
	cursor_y = y;
	infobar->cursor_y = cursor_y;
	infobar->update_texture();
	update_mark();
}

void Buffer::set_cursor_x(int x) {
	cursor_x = x;
	update_mark();
}

bool Buffer::is_line_empty() {
	return (lines[cursor_y]->text == "") ||
		(lines[cursor_y]->text.find_first_not_of(' ') == std::string::npos);
}

void Buffer::update_view() {
	if (cursor_x * char_width - view_x >= window_dim->width ||
		cursor_x * char_width - view_x < 0) {
		view_x = cursor_x * char_width + char_width - window_dim->width;
	}

	if (cursor_y * char_height - view_y > window_dim->height - char_height * 3 ||
		cursor_y * char_height - view_y < 0) {
		view_y = cursor_y * char_height - window_dim->height / 2;
	}

	if (view_x < 0) view_x = 0;
	if (view_y < 0) view_y = 0;
}

void Buffer::center_view() {
	view_y = cursor_y * char_height - window_dim->height / 2;
	if (view_y < 0) view_y = 0;
}

void Buffer::type(const char *text) {
	if (is_mark_open) {
		kill_mark();	
	}

	if (overwrite_mode) {
		lines[cursor_y]->text.replace(lines[cursor_y]->text.begin()+cursor_x,
									  lines[cursor_y]->text.begin()+cursor_x+strlen(text),
									  text);
		lines[cursor_y]->update_texture();
	} else {
		lines[cursor_y]->add_chars(cursor_x, text);
	}

	if (!is_minibuffer) {
		infobar->set_has_edited(true);
		infobar->update_texture();
	}

	cursor_move_right();
}

void Buffer::minibuffer_clear() {
	lines[0]->text = "";
	lines[0]->prefix = "";
	lines[0]->update_texture();
		
	set_cursor_x(0);
}

void Buffer::view_down() {
	view_y += ((int) (window_dim->height / char_height) * char_height) - char_height * 5;
	if (view_y / char_height > lines.size()) {
		view_y = lines.size() * char_height - (window_dim->height / 2);
	}
	cursor_y = (int) (view_y / char_height);
}

void Buffer::view_up() {
	view_y -= ((int) (window_dim->height / char_height) * char_height) + char_height * 5;
	if (view_y < 0) view_y = 0;
				
	cursor_y = (int) (view_y / char_height);
}

bool Buffer::is_meta_pressed(const Uint8 *keyboard) {
	return keyboard[SDL_SCANCODE_LALT] || keyboard[SDL_SCANCODE_RALT];
}

bool Buffer::is_ctrl_pressed(const Uint8 *keyboard) {
	return keyboard[SDL_SCANCODE_LCTRL] || keyboard[SDL_SCANCODE_RCTRL];
}

void Buffer::kill_line(const Uint8 *keyboard) {
	Line *line = lines[cursor_y];
	if (is_ctrl_pressed(keyboard)) {
		if (line->text == "" && lines.size() > 1) {
			lines.erase(lines.begin() + cursor_y);
			return;
		}
		line->text = line->text.substr(0, cursor_x);
		line->update_texture();
	}
}

void Buffer::backspace(const Uint8 *keyboard) {
	if (is_mark_open) {
		kill_mark();
		return;
	}
	
	if (cursor_x == 0 && cursor_y == 0) return;

	if (is_ctrl_pressed(keyboard)) {
		delete_previous_word();
	} else {
		if (cursor_x == 0 && cursor_y > 0) {
			std::string line_str = lines[cursor_y]->text;

			lines.erase(lines.begin() + cursor_y);
			cursor_move_up();
			set_cursor_x(lines[cursor_y]->text.size());
			lines[cursor_y]->text += line_str;
			lines[cursor_y]->update_texture();
		} else {
			lines[cursor_y]->remove_char(cursor_x);
			cursor_move_left();
		}
	}
}

void Buffer::event_update(const SDL_Event &event) {
	if (!in_focus) return;

	Line *line = lines[cursor_y];

	const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
	
	if (event.type == SDL_TEXTINPUT) {
		if (strcmp(event.text.text, " ") == 0 && is_ctrl_pressed(keyboard)) {
			mark_start();
		} else {
			type(event.text.text);
		}
	} else if (event.type == SDL_KEYDOWN) {
		// Clear minibuffer when any key is pressed.
		if (!is_minibuffer && mini_buffer->cursor_x > 0) {
			mini_buffer->minibuffer_clear();
		}
		
		switch (event.key.keysym.sym) {
		case SDLK_F11: {
			if (infobar->text[1] == ':') {
				std::string s = "start cmd.exe /K \"";
				s += infobar->text.substr(0, 2);
				s += " && cd ";
				size_t pos = infobar->text.find_last_of('\\');
				std::string t = infobar->text.substr(0, pos + 1);
				s += t + "\"";
				printf("%s\n", &s[0]);
				system(&s[0]);
			} else {
				system("start");
			}
			break;
		}
		case SDLK_F5: {
			if (is_minibuffer) break;
			
			if (compile_command == "") {
				mini_buffer->lines[0]->text = "Compilation command not set. Press f10 to set.";
				mini_buffer->lines[0]->prefix = "";
				mini_buffer->lines[0]->update_texture();
				mini_buffer->cursor_x = mini_buffer->lines[0]->text.size();
				break;
			}
			
			if (infobar->text[1] == ':') {
				std::string s = "start cmd.exe /K \"";
				s += infobar->text.substr(0, 2);
				s += " && cd ";
				size_t pos = infobar->text.find_last_of('\\');
				std::string t = infobar->text.substr(0, pos + 1);
				s += t + " && " + compile_command + "\"";
				system(&s[0]);
			} else {
				std::string s = "start && ";
				s += compile_command;
				system(&s[0]);
			}

			break;
		}
		case SDLK_F6: {
			if (is_minibuffer) break;
			
			if (program_executable == "") {
				mini_buffer->lines[0]->text = "Exeuctable name not set. Press f12 to set.";
				mini_buffer->lines[0]->prefix = "";
				mini_buffer->lines[0]->update_texture();
				mini_buffer->cursor_x = mini_buffer->lines[0]->text.size();
				break;
			}
			
			if (infobar->text[1] == ':') {
				std::string s = "start cmd.exe /K \"";
				s += infobar->text.substr(0, 2);
				s += " && cd ";
				size_t pos = infobar->text.find_last_of('\\');
				std::string t = infobar->text.substr(0, pos + 1);
				s += t + " && " + program_executable + "\"";
				system(&s[0]);
			} else {
				std::string s = "start && ";
				s += program_executable;
				system(&s[0]);
			}
			
			break;
		}
		case SDLK_v: {
			if (is_ctrl_pressed(keyboard)) {
				view_down();
			} else if (is_meta_pressed(keyboard)) {
				view_up();
			}
			break;
		}
		case SDLK_BACKSPACE: {
			backspace(keyboard);
			break;
		}
		case SDLK_RETURN: {
			if (is_minibuffer) {
				switch (mode) {
				case MB_SaveFile: {
					if (main_buffer == nullptr) {
						fprintf(stderr, "buffer ptr inside minibuffer not set\n");
						exit(1);
					}

					std::ofstream stream(line->text);
					for (auto *l : main_buffer->lines) {
						std::string text = replace_string(l->text,
														  "    ",
														  "\t");
						
						stream << text << "\n";
					}
					stream.close();

					infobar->text = line->text;
					infobar->set_has_edited(false);
					infobar->update_texture();

					line->prefix = "";
					line->text = "Saved to " + line->text + ".";
					cursor_x = line->text.size();
					line->update_texture();

					in_focus = false;
					main_buffer->in_focus = true;
					break;
				}
				case MB_LoadFile: {
					int err = main_buffer->load_from_file(&(lines[0]->text[0]));
					line->prefix = "";
					if (err) {
						for (auto *e : main_buffer->lines) { delete e; }
						main_buffer->lines.clear();
						
						main_buffer->lines.push_back(new Line(renderer, font));
						main_buffer->lines.back()->update_texture();

						infobar->text = lines[0]->text;
						infobar->update_texture();

						line->text = "Created new file.";
					} else {
						line->text = lines[0]->text + " loaded successfully.";
					}
					cursor_x = line->text.size();
					line->update_texture();

					in_focus = false;
					infobar->set_has_edited(false);
					main_buffer->in_focus = true;
					view_y = 0;
					
					main_buffer->set_cursor_x(0);
					main_buffer->set_cursor_y(0);
					break;
				}
				case MB_GotoLine: {
					try {
						int linenum = std::stoi(line->text);

						if (linenum < 0) {
							linenum = 0;
						} else if (linenum > main_buffer->lines.size()-1) {
							linenum = main_buffer->lines.size() - 1;
						}
						
						main_buffer->set_cursor_y(linenum);
						main_buffer->set_cursor_x(0);
						line->text = "Cursor set.";

						main_buffer->update_view();
					} catch (std::invalid_argument) {
						line->text = "Invalid number.";
					}

					line->prefix = "";
					cursor_x = line->text.size();
					line->update_texture();

					in_focus = false;
					main_buffer->in_focus = true;
					break;
				}
				case MB_SetCompileCommand: {
					main_buffer->compile_command = line->text;

					line->prefix = "";
					line->text = "Command set.";
					line->update_texture();

					cursor_x = line->text.size();

					in_focus = false;
					main_buffer->in_focus = true;
					break;
				}
				case MB_SetProgramExecutable: {
					main_buffer->program_executable = line->text;

					line->prefix = "";
					line->text = "Executable name set.";
					line->update_texture();

					cursor_x = line->text.size();

					in_focus = false;
					main_buffer->in_focus = true;
					break;
				}
				}
				break;
			} // End if is_minibuffer
			
			std::string text_after = line->text.substr(cursor_x,
													   line->text.size() -
													   cursor_x);
			
			lines[cursor_y]->text = line->text.substr(0, cursor_x);
			lines[cursor_y]->update_texture();
			
			lines.insert(lines.begin() + cursor_y + 1, new Line(renderer, font));
			cursor_move_down();
			
			set_cursor_x(0);
			
			lines[cursor_y]->text += text_after;
			lines[cursor_y]->update_texture();
			break;
		}
		// case SDLK_w: {
		// 	if (is_meta_pressed(keyboard)) {
		// 		if (clip) {
		// 			delete clip;
		// 			clip = new Clipboard;
		// 		}
		// 		clip->copy_text("weee", 4);
		// 	}
		// 	break;
		// }
		// case SDLK_y: {
		// 	if (keyboard[SDL_SCANCODE_LCTRL]) {
		// 		if (!clip) {
		// 			mini_buffer->lines[0]->text = "No text selected.";
		// 			mini_buffer->lines[0]->update_texture();
		// 			mini_buffer->cursor_x = mini_buffer->lines[0]->text.size();
		// 		} else {
		// 			clip->paste_text();
		// 		}
		// 	}
		// 	break;
		// }
		case SDLK_INSERT: {
			toggle_overwrite_mode();
			break;
		}
		case SDLK_i: {
			if (is_ctrl_pressed(keyboard)) {
				toggle_overwrite_mode();
			}
			break;
		}
		case SDLK_d: {
			if (is_ctrl_pressed(keyboard)) {
				if (cursor_x + 1 > line->text.size()) break;
				line->remove_char(cursor_x + 1);
			}
			break;
		}
		case SDLK_COMMA: {
			if (is_meta_pressed(keyboard) && keyboard[SDL_SCANCODE_LSHIFT]) {
				set_cursor_x(0);
				set_cursor_y(0);
				update_view();
			}
			break;
		}
		case SDLK_PERIOD: {
			if (is_meta_pressed(keyboard) && keyboard[SDL_SCANCODE_LSHIFT]) {
				set_cursor_y(lines.size()-1);
				set_cursor_x(lines[cursor_y]->text.size());
				update_view();
			}
			break;
		}
		case SDLK_p: {
			if (is_ctrl_pressed(keyboard)) {
				cursor_move_up();
			}
			break;
		}
		case SDLK_n: {
			if (is_ctrl_pressed(keyboard)) {
				cursor_move_down();
			}
			break;
		}
		case SDLK_b: {
			if (is_ctrl_pressed(keyboard)) {
				cursor_move_left();
			} else if (is_meta_pressed(keyboard)) {
				cursor_backward_word();
			}
			break;
		}
		case SDLK_f: {
			if (is_ctrl_pressed(keyboard)) {
				cursor_move_right();
			} else if (is_meta_pressed(keyboard)) {
				cursor_forward_word();
			}
			break;
		}
		case SDLK_e: {
			if (is_ctrl_pressed(keyboard)) {
				cursor_x = line->text.size();
			}
			break;
		}
		case SDLK_a: {
			if (is_ctrl_pressed(keyboard)) {
				set_cursor_x(0);
			}
			break;
		}
		case SDLK_l: {
			if (is_ctrl_pressed(keyboard)) {
				center_view();
			}
			break;
		}
		case SDLK_k: {
			kill_line(keyboard);
			break;
		}
		case SDLK_TAB: {
			line->text.insert(cursor_x, "    ");
			line->update_texture();
			cursor_x += 4;
			break;
		}
		case SDLK_F10: {
			if (is_minibuffer) break;
			mini_buffer->mode = MB_SetCompileCommand;
			mini_buffer->lines[0]->prefix = "Compile Command: ";
			mini_buffer->lines[0]->update_texture();

			mini_buffer->in_focus = true;
			in_focus = false;
			break;
		}
		case SDLK_F12: {
			if (is_minibuffer) break;
			mini_buffer->mode = MB_SetProgramExecutable;
			mini_buffer->lines[0]->prefix = "Program Executable: ";
			mini_buffer->lines[0]->update_texture();

			mini_buffer->in_focus = true;
			in_focus = false;
			break;
		}
		case SDLK_s: {
			if (is_minibuffer) break;
			
			if (is_ctrl_pressed(keyboard)) {
				if (mini_buffer == nullptr) {
					fprintf(stderr, "mini buffer ptr not set in buffer.");
					exit(1);
				}

				mini_buffer->mode = MB_SaveFile;
				mini_buffer->lines[0]->prefix = "Save to: ";
				if (infobar->text != "*buffer*") {
					mini_buffer->lines[0]->text = infobar->text;
					mini_buffer->cursor_x = mini_buffer->lines[0]->text.size();
				}
				mini_buffer->lines[0]->update_texture();

				mini_buffer->in_focus = true;
				in_focus = false;
			}
			break;
		}
		case SDLK_c: {
			if (is_minibuffer) break;
			
			if (is_ctrl_pressed(keyboard)) {
				if (mini_buffer == nullptr) {
					fprintf(stderr, "mini buffer ptr not set in buffer.");
					exit(1);
				}

				mini_buffer->mode = MB_LoadFile;
				mini_buffer->lines[0]->prefix = "Load File: ";
				if (infobar->text[1] == ':') {
					size_t pos = infobar->text.find_last_of('\\');
					mini_buffer->lines[0]->text = infobar->text.substr(0, pos + 1);
					mini_buffer->cursor_x = mini_buffer->lines[0]->text.size();
				} else {
					mini_buffer->lines[0]->text = "";
				}
				mini_buffer->lines[0]->update_texture();
				mini_buffer->in_focus = true;
				in_focus = false;
			}
			break;
		}
		case SDLK_g: {
			if (is_ctrl_pressed(keyboard)) mark_end();
			
			if (is_minibuffer) {
				if (is_ctrl_pressed(keyboard)) {
					lines[0]->text = "";
					lines[0]->update_texture();
					main_buffer->in_focus = true;
					in_focus = false;
				}
			} else {
				if (is_meta_pressed(keyboard)) {
					mini_buffer->mode = MB_GotoLine;
					mini_buffer->lines[0]->prefix = "Goto Line: ";
					mini_buffer->lines[0]->text = "";
					mini_buffer->lines[0]->update_texture();
					mini_buffer->in_focus = true;
					in_focus = false;
				}
			}
			break;
		}
		case SDLK_LEFTBRACKET: {
			if (is_meta_pressed(keyboard) && keyboard[SDL_SCANCODE_LSHIFT]) {
				if (is_line_empty()) cursor_y--;
				
				while (cursor_y > 0 && !is_line_empty()) {
					cursor_move_up();
				}
			}
			break;
		}
		case SDLK_RIGHTBRACKET: {
			if (is_meta_pressed(keyboard) && keyboard[SDL_SCANCODE_LSHIFT]) {
				if (is_line_empty()) cursor_y++;
				
				while (cursor_y < lines.size()-1 && !is_line_empty()) {
					cursor_move_down();
				}
			}
			break;
		}
		}
	}

	update_mark();
	
	update_view();
	clamp_cursor();
}

void Buffer::render() {
	int yoff = 0;
	if (is_minibuffer) {
		yoff = window_dim->height - char_height;
		
		const SDL_Rect rect = { 0,
								window_dim->height - char_height,
								window_dim->width,
								char_height };
		
		SDL_SetRenderDrawColor(renderer, 6, 35, 41, 255);
		SDL_RenderFillRect(renderer, &rect);
	}
	
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	for (size_t i = 0; i < lines.size(); ++i) {
		lines[i]->render(-view_x, i * char_height + yoff - view_y);
	}
}

void Buffer::clamp_cursor() {
	if (cursor_y < 0) set_cursor_y(0);
	else if (cursor_y > lines.size() - 1) set_cursor_y(lines.size() - 1);
	else if (cursor_x < 0) set_cursor_x(0);
	else if (cursor_x > lines[cursor_y]->text.size()) set_cursor_x(lines[cursor_y]->text.size());
}

void Buffer::render_cursor() {
	int yoff = -view_y;
	if (is_minibuffer) yoff += window_dim->height - char_height;
	int xoff = -view_x;
	
	SDL_Rect cursor = { cursor_x * char_width + xoff,
						cursor_y * char_height + yoff,
						char_width,
						char_height };

	if (is_minibuffer) {
		cursor.x += lines[0]->prefix.size() * char_width;
	}

	Uint32 windowflags = SDL_GetWindowFlags(window);

	if (overwrite_mode) {
		SDL_SetRenderDrawColor(renderer, 255, 255, 200, 200);
		SDL_RenderFillRect(renderer, &cursor);
		return;
	}
	
	if (!in_focus || !(windowflags & SDL_WINDOW_INPUT_FOCUS)) {
		SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
		SDL_RenderFillRect(renderer, &cursor);
	} else {
		if (is_mark_open) {
			SDL_SetRenderDrawColor(renderer, 37, 241, 252, 255);
		} else {
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		}
		SDL_RenderDrawRect(renderer, &cursor);
	}
}
