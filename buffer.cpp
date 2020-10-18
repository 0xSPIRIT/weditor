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

bool Buffer::load_from_file(const char *fp) {
	std::ifstream file(fp);
	if (!file.is_open()) {
		return 1;
	}

	for (auto *e : lines) { delete e; }
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

	return 0;
}

Buffer::~Buffer() {
	for (auto *l : lines) {
		delete l;
	}
}

void Buffer::cursor_move_up() {
	cursor_y--;
	infobar->cursor_y = cursor_y;
	infobar->update_texture();

	if (cursor_y * char_height - view_y < 0) {
		view_y -= char_height * (int) (window_dim->height / 2 / char_height);
	}
	if (view_y < 0) view_y = 0;
}

void Buffer::cursor_move_down() {
	cursor_y++;
	infobar->cursor_y = cursor_y;
	infobar->update_texture();

	if (cursor_y * char_height + char_height * 3 - view_y > window_dim->height) {
		view_y += char_height * (int) (window_dim->height / 2 / char_height);
	}
}

void Buffer::cursor_move_left() {
	cursor_x--;
}

void Buffer::cursor_move_right() {
	cursor_x++;
}

void Buffer::cursor_forward_word() {
	if (cursor_x == lines[cursor_y]->text.size()) return;

	Line *line = lines[cursor_y];
				
	if (line->text[cursor_x] == ' '	||
		line->text[cursor_x] != '('	||
		line->text[cursor_x] != ')'	||
		line->text[cursor_x] != '['	||
		line->text[cursor_x] != ']'	||
		line->text[cursor_x] != '_') {
		cursor_x++;
	}
	while (line->text[cursor_x] != ' '	&&
		   cursor_x < line->text.size()-1 &&
		   line->text[cursor_x] != '('	&&
		   line->text[cursor_x] != ')'	&&
		   line->text[cursor_x] != '['	&&
		   line->text[cursor_x] != ']'	&&
		   line->text[cursor_x] != '_') {
		cursor_x++;
	}
	cursor_x++;
}

void Buffer::cursor_backward_word() {
	if (cursor_x == 0) return;
				
	Line *line = lines[cursor_y];
				
	if (line->text[cursor_x] != ' '	||
		line->text[cursor_x] != '('	||
		line->text[cursor_x] != ')'	||
		line->text[cursor_x] != '['	||
		line->text[cursor_x] != ']'	||
		line->text[cursor_x] != '_') {
		cursor_x--;
	}
				
	while (line->text[cursor_x] != ' ' &&
		   cursor_x  != 0   &&
		   line->text[cursor_x] != '(' &&
		   line->text[cursor_x] != ')' &&
		   line->text[cursor_x] != '[' &&
		   line->text[cursor_x] != ']' &&
		   line->text[cursor_x] != '_') {
		cursor_x--;
	}
}

void Buffer::set_cursor_y(int y) {
	cursor_y = y;
	infobar->cursor_y = cursor_y;
	infobar->update_texture();
}

void Buffer::set_cursor_x(int x) {
	cursor_x = x;
}

bool Buffer::is_line_empty() {
	return (lines[cursor_y]->text == "") ||
		(lines[cursor_y]->text.find_first_not_of(' ') == std::string::npos);
}

void Buffer::update_view() {
	if (cursor_y * char_height - view_y > window_dim->height - char_height * 3 ||
		cursor_y * char_height - view_y < 0) {
		view_y = cursor_y * char_height - window_dim->height / 2;
	}
	if (view_y < 0) view_y = 0;
}

void Buffer::center_view() {
	view_y = cursor_y * char_height - window_dim->height / 2;
	if (view_y < 0) view_y = 0;
}

void Buffer::event_update(const SDL_Event &event) {
	if (!in_focus) return;

	Line *line = lines[cursor_y];

	const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
	
	if (event.type == SDL_TEXTINPUT) {
		line->add_chars(cursor_x, event.text.text);
		cursor_move_right();
	} else if (event.type == SDL_KEYDOWN) {
		switch (event.key.keysym.sym) {
		case SDLK_v: {
			if (keyboard[SDL_SCANCODE_LCTRL]) {
				view_y += (int) (window_dim->height - 100 / char_height);
				if (view_y / char_height > lines.size()) {
					view_y = lines.size() * char_height - (window_dim->height / 2);
				}
				cursor_y = (int) ((view_y / char_height) + 1);
			} else if (keyboard[SDL_SCANCODE_LALT]) {
				view_y -= (int) (window_dim->height - 100 / char_height);
				if (view_y < 0) view_y = 0;
				
				cursor_y = (int) ((view_y / char_height) + 1);
			}
			break;
		}
		case SDLK_BACKSPACE: {
			if (cursor_x == 0 && cursor_y == 0) break;
			
			if (cursor_x == 0 && cursor_y > 0) {
				std::string line_str = line->text;
				
				lines.erase(lines.begin() + cursor_y);
				cursor_move_up();
				cursor_x = lines[cursor_y]->text.size();
				lines[cursor_y]->text += line_str;
				lines[cursor_y]->update_texture();
			} else {
				line->remove_char(cursor_x);
				cursor_move_left();
			}
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

					std::string title = line->text + " - weditor";
					SDL_SetWindowTitle(window, &title[0]);

					infobar->text = line->text;
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
					main_buffer->in_focus = true;
					view_y = 0;
					
					main_buffer->cursor_x = 0;
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
						main_buffer->cursor_x = 0;
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
				}
				break;
			}
			
			std::string text_after = line->text.substr(cursor_x,
													   line->text.size() -
													   cursor_x);
			
			lines[cursor_y]->text = line->text.substr(0, cursor_x);
			lines[cursor_y]->update_texture();
			
			lines.insert(lines.begin() + cursor_y + 1, new Line(renderer, font));
			cursor_move_down();
			
			cursor_x = 0;
			
			lines[cursor_y]->text += text_after;
			lines[cursor_y]->update_texture();
			break;
		}
		case SDLK_d: {
			if (keyboard[SDL_SCANCODE_LCTRL]) {
				if (cursor_x + 1 > line->text.size()) break;
				line->remove_char(cursor_x + 1);
			}
			break;
		}
		case SDLK_COMMA: {
			if (keyboard[SDL_SCANCODE_LALT] && keyboard[SDL_SCANCODE_LSHIFT]) {
				cursor_x = 0;
				set_cursor_y(0);
				update_view();
			}
			break;
		}
		case SDLK_PERIOD: {
			if (keyboard[SDL_SCANCODE_LALT] && keyboard[SDL_SCANCODE_LSHIFT]) {
				set_cursor_y(lines.size()-1);
				cursor_x = lines[cursor_y]->text.size();
				update_view();
			}
			break;
		}
		case SDLK_p: {
			if (keyboard[SDL_SCANCODE_LCTRL]) {
				cursor_move_up();
			}
			break;
		}
		case SDLK_n: {
			if (keyboard[SDL_SCANCODE_LCTRL]) {
				cursor_move_down();
			}
			break;
		}
		case SDLK_b: {
			if (keyboard[SDL_SCANCODE_LCTRL]) {
				cursor_move_left();
			} else if (keyboard[SDL_SCANCODE_RALT]) {
				cursor_backward_word();
			}
			break;
		}
		case SDLK_f: {
			if (keyboard[SDL_SCANCODE_LCTRL]) {
				cursor_move_right();
			} else if (keyboard[SDL_SCANCODE_RALT]) {
				cursor_forward_word();
			}
			break;
		}
		case SDLK_e: {
			if (keyboard[SDL_SCANCODE_LCTRL]) {
				cursor_x = line->text.size();
			}
			break;
		}
		case SDLK_a: {
			if (keyboard[SDL_SCANCODE_LCTRL]) {
				cursor_x = 0;
			}
			break;
		}
		case SDLK_l: {
			if (keyboard[SDL_SCANCODE_LCTRL]) {
				center_view();
			}
			break;
		}
		case SDLK_k: {
			if (keyboard[SDL_SCANCODE_LCTRL]) {
				if (line->text == "" && cursor_y > 0) {
					lines.erase(lines.begin() + cursor_y);
					break;
				}
				line->text = line->text.substr(0, cursor_x);
				line->update_texture();
			}
			break;
		}
		case SDLK_TAB: {
			line->text.insert(cursor_x, "    ");
			line->update_texture();
			cursor_x += 4;
			break;
		}
		case SDLK_s: {
			if (is_minibuffer) break;
			
			if (keyboard[SDL_SCANCODE_LCTRL]) {
				if (mini_buffer == nullptr) {
					fprintf(stderr, "mini buffer ptr not set in buffer.");
					exit(1);
				}

				mini_buffer->mode = MB_SaveFile;
				mini_buffer->lines[0]->prefix = "Save to: ";
				if (infobar->text != "*buffer*") {
					mini_buffer->lines[0]->text = infobar->text;
				}
				mini_buffer->lines[0]->update_texture();

				mini_buffer->in_focus = true;
				in_focus = false;
			}
			break;
		}
		case SDLK_c: {
			if (is_minibuffer) break;
			
			if (keyboard[SDL_SCANCODE_LCTRL]) {
				if (mini_buffer == nullptr) {
					fprintf(stderr, "mini buffer ptr not set in buffer.");
					exit(1);
				}

				mini_buffer->mode = MB_LoadFile;
				mini_buffer->lines[0]->prefix = "Load File: ";
				mini_buffer->lines[0]->text = "";
				mini_buffer->lines[0]->update_texture();
				mini_buffer->in_focus = true;
				in_focus = false;
			}
			break;
		}
		case SDLK_g: {
			if (is_minibuffer) {
				if (keyboard[SDL_SCANCODE_LCTRL]) {
					lines[0]->text = "";
					lines[0]->update_texture();
					main_buffer->in_focus = true;
					in_focus = false;
				}
			} else {
				if (keyboard[SDL_SCANCODE_RALT]) {
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
			if (keyboard[SDL_SCANCODE_LALT] && keyboard[SDL_SCANCODE_LSHIFT]) {
				if (is_line_empty()) cursor_y--;
				
				while (cursor_y > 0 && !is_line_empty()) {
					cursor_move_up();
				}
			}
			break;
		}
		case SDLK_RIGHTBRACKET: {
			if (keyboard[SDL_SCANCODE_LALT] && keyboard[SDL_SCANCODE_LSHIFT]) {
				if (is_line_empty()) cursor_y++;
				
				while (cursor_y < lines.size()-1 && !is_line_empty()) {
					cursor_move_down();
				}
			}
			break;
		}
		}
	}

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
		lines[i]->render(i * char_height + yoff - view_y);
	}
}

void Buffer::clamp_cursor() {
	if (cursor_y < 0) set_cursor_y(0);
	else if (cursor_y > lines.size() - 1) set_cursor_y(lines.size() - 1);
	else if (cursor_x < 0) cursor_x = 0;
	else if (cursor_x > lines[cursor_y]->text.size()) cursor_x = lines[cursor_y]->text.size();
}

void Buffer::render_cursor() {
	int yoff = -view_y;
	if (is_minibuffer) yoff += window_dim->height - char_height;
	
	SDL_Rect cursor = { cursor_x * char_width,
						cursor_y * char_height + yoff,
						char_width,
						char_height };

	if (is_minibuffer) {
		cursor.x += lines[0]->prefix.size() * char_width;
	}

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	if (in_focus) {
		SDL_RenderFillRect(renderer, &cursor);
	} else {
		SDL_RenderDrawRect(renderer, &cursor);
	}
}
