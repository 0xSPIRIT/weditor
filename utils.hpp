#pragma once

std::string replace_string(std::string subject, const std::string& search,
						   const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

void replace_string_in_place(std::string& subject, const std::string& search,
							 const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
}

int sign(int num) {
	return (num == 0 ? num : (num > 0 ? 1 : -1));
}
