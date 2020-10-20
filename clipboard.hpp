#pragma once

#include <stdlib.h>
#include <string>

struct Clipboard {
	Clipboard();
	~Clipboard();

	void copy_text(const char *text, size_t len);
	std::string paste_text();
};
