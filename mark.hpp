#pragma once

#include <stdlib.h>

struct Mark {
	Buffer *buffer;
	size_t start_x, start_y, end_x, end_y;

	bool on;

	Mark();
	~Mark();

	void event_update(SDL_Event *event);
};
