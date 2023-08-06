#ifndef FISH_ICONV_MAC_H
#define FISH_ICONV_MAC_H

#include <string>

int iconv_open_utf8mac(void);
size_t iconv_utf8mac(std::string * dst_path, const char *src, size_t len);

#endif
