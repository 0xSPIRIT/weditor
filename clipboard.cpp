#include "clipboard.hpp"

#include <iostream>
#include <windows.h>

Clipboard::Clipboard() {
	if (!OpenClipboard(0)) {
		fprintf(stderr, "CLIPBOARD COULD NOT BE OPENED!\n");
	}
}

Clipboard::~Clipboard() {
	CloseClipboard();
}

void Clipboard::copy_text(const char *text, size_t len) {
	if (!EmptyClipboard()) {
		fprintf(stderr, "Cannot empty clipboard.");
	}

	HGLOBAL global = GlobalAlloc(GMEM_FIXED, (len + 1));
	strncpy((char *) global, text, len);
	SetClipboardData(CF_TEXT, global);

	GlobalFree(global);
}

std::string Clipboard::paste_text() {
	char* result = (char*) GetClipboardData(CF_TEXT);
	puts(result);

	if (result == NULL) {
		fprintf(stderr, "Clipboard has no data to paste.");
		return "";
	}

	GlobalFree(result);

	return result;
}

