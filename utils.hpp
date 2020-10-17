#pragma once

/* TODO: Improve performance- this function is used every
         time a file is saved or loaded. Causes a slight 
		 bit of lag. Will be worse for larger files.
 */

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
