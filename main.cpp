#include <string>
#include <unistd.h>
#include <libgen.h>
#include <cwchar>
#include <clocale>
#include <climits>
#include "iconv_mac.h"

#define BUFSIZE 1024

#define ENCODE_DIRECT_BASE static_cast<wchar_t>(0xF600)
#define RESERVED_CHAR_BASE static_cast<wchar_t>(0xFDD0)
#define EXPAND_RESERVED_BASE RESERVED_CHAR_BASE

enum : wchar_t {
    /// Character representing a home directory.
    HOME_DIRECTORY = EXPAND_RESERVED_BASE,
    /// Character representing process expansion for %self.
    PROCESS_EXPAND_SELF,
    /// Character representing variable expansion.
    VARIABLE_EXPAND,
    /// Character representing variable expansion into a single element.
    VARIABLE_EXPAND_SINGLE,
    /// Character representing the start of a bracket expansion.
    BRACE_BEGIN,
    /// Character representing the end of a bracket expansion.
    BRACE_END,
    /// Character representing separation between two bracket elements.
    BRACE_SEP,
    /// Character that takes the place of any whitespace within non-quoted text in braces
    BRACE_SPACE,
    /// Separate subtokens in a token with this character.
    INTERNAL_SEPARATOR,
    /// Character representing an empty variable expansion. Only used transitively while expanding
    /// variables.
    VARIABLE_EXPAND_EMPTY,
    /// This is a special pseudo-char that is not used other than to mark the end of the the special
    /// characters so we can sanity check the enum range.
    EXPAND_SENTINEL
};



// #define ENCODE_DIRECT_END (ENCODE_DIRECT_BASE + 256)
std::wstring str2wcstring(const char *in);
std::wstring str2wcstring(const std::string &in);

// static std::wstring str2wcs_internal(const char *in, const size_t in_len);

std::string wcs2string(const wchar_t *in, size_t len);

int main(int argc, char ** argv) {

    iconv_open_utf8mac();

    char buf[BUFSIZE];
    char * name = basename(getcwd(buf, BUFSIZE));
    std::wstring wstr = str2wcstring(name);
    std::string  cstr = wcs2string(wstr.c_str(), wstr.length());

    std::string cstr_out_utf8;
    iconv_utf8mac(&cstr_out_utf8, cstr.c_str(), cstr.length());

    std::string cstr_out_utf8_2;
    iconv_utf8mac(&cstr_out_utf8_2, name, strlen(name));

    std::wstring wstr_out_utf8 = str2wcstring(cstr_out_utf8);
    std::wstring wstr_out_utf8_2 = str2wcstring(cstr_out_utf8_2);

    const char * p = reinterpret_cast<const char *>(wstr.data());
    wprintf(L"wstr as in bytes => %-30s\n", wstr.c_str());
    for ( size_t i = 0; i < (wstr.length()) * sizeof(wchar_t); ++i ) {
        printf("0%hhX ", *(p + i));
    }
    printf("\n\n");

    wprintf(L"wstr => %-30s\n", wstr.c_str());
    for ( size_t i = 0; i < wstr.length(); ++i ) {
        wprintf(L"0%hhX ", wstr[i]);
    }
    printf("\n\n");

    printf("cstr => %-30s\n", cstr.c_str());
    for ( size_t i = 0; i < cstr.length(); ++i ) {
        printf("0%hhX ", cstr[i]);
    }
    printf("\n\n");

    printf("cstr_out_utf8 => %-30s\n", cstr_out_utf8.c_str());
    for ( size_t i = 0; i < cstr_out_utf8.length(); ++i ) {
        printf("0%hhX ", cstr_out_utf8[i]);
    }
    printf("\n\n");

    wprintf(L"wstr_out_utf8 => %-30s\n", wstr_out_utf8.c_str());
    for ( size_t i = 0; i < wstr_out_utf8.length(); ++i ) {
        wprintf(L"0%hhX ", wstr_out_utf8[i]);
    }
    printf("\n\n");

    wprintf(L"wstr_out_utf8_2 => %-30s\n", wstr_out_utf8_2.c_str());
    for ( size_t i = 0; i < wstr_out_utf8_2.length(); ++i ) {
        wprintf(L"0%hhX ", wstr_out_utf8_2[i]);
    }
    printf("\n\n");

    // std::string nfd = "한글";
    // std::string nfc = "한글";

    // bool equal_to_nfd = strcmp(currend_dir_name.c_str(), nfd.c_str()) == 0;
    // bool equal_to_nfc = strcmp(currend_dir_name.c_str(), nfc.c_str()) == 0;

    // printf("basename == nfd ? %s\n", equal_to_nfd ? "true" : "false");
    // printf("basename == nfc ? %s\n", equal_to_nfc ? "true" : "false");

    // return 0;
}

/// \return the smallest pointer in the range [start, start + len] which is aligned to Align.
/// If there is no such pointer, return \p start + len.
/// alignment must be a power of 2 and in range [1, 64].
/// This is intended to return the end point of the "unaligned prefix" of a vectorized loop.
template <size_t Align>
inline const char *align_start(const char *start, size_t len) {
    static_assert(Align >= 1 && Align <= 64, "Alignment must be in range [1, 64]");
    static_assert((Align & (Align - 1)) == 0, "Alignment must be power of 2");
    uintptr_t startu = reinterpret_cast<uintptr_t>(start);
    // How much do we have to add to start to make it 0 mod Align?
    // To compute 17 up-aligned by 8, compute its skew 17 % 8, yielding 1,
    // and then we will add 8 - 1. Of course if we align 16 with the same idea, we will
    // add 8 instead of 0, so then mod the summand by Align again.
    // Note all of these mods are optimized to masks.
    uintptr_t add_which_aligns = Align - (startu % Align);
    add_which_aligns %= Align;
    // Add that much but not more than len. If we add 'add_which_aligns' we may overflow the
    // pointer.
    return start + std::min(static_cast<size_t>(add_which_aligns), len);
}

/// \return the largest pointer in the range [start, start + len] which is aligned to Align.
/// If there is no such pointer, return \p start.
/// This is intended to be the start point of the "unaligned suffix" of a vectorized loop.
template <size_t Align>
inline const char *align_end(const char *start, size_t len) {
    static_assert(Align >= 1 && Align <= 64, "Alignment must be in range [1, 64]");
    static_assert((Align & (Align - 1)) == 0, "Alignment must be power of 2");
    // How much do we have to subtract to align it? Its value, mod Align.
    uintptr_t endu = reinterpret_cast<uintptr_t>(start + len);
    uintptr_t sub_which_aligns = endu % Align;
    return start + len - std::min(static_cast<size_t>(sub_which_aligns), len);
}

static size_t count_ascii_prefix(const char *in, size_t in_len) {
    // We'll use aligned reads of this type.
    using WordType = uint32_t;
    const char *aligned_start = align_start<alignof(WordType)>(in, in_len);
    const char *aligned_end = align_end<alignof(WordType)>(in, in_len);

    // Consume the unaligned prefix.
    for (const char *cursor = in; cursor < aligned_start; cursor++) {
        if (cursor[0] & 0x80) return &cursor[0] - in;
    }

    // Consume the aligned middle.
    for (const char *cursor = aligned_start; cursor < aligned_end; cursor += sizeof(WordType)) {
        if (*reinterpret_cast<const WordType *>(cursor) & 0x80808080) {
            if (cursor[0] & 0x80) return &cursor[0] - in;
            if (cursor[1] & 0x80) return &cursor[1] - in;
            if (cursor[2] & 0x80) return &cursor[2] - in;
            return &cursor[3] - in;
        }
    }

    // Consume the unaligned suffix.
    for (const char *cursor = aligned_end; cursor < in + in_len; cursor++) {
        if (cursor[0] & 0x80) return &cursor[0] - in;
    }
    return in_len;
}

static std::wstring str2wcs_internal(const char *in, const size_t in_len) {
    if (in_len == 0) return std::wstring();
    assert(in != nullptr);
    printf("str2wcs_internal \n");

    std::wstring result;
    result.reserve(in_len);

    // In the unlikely event that MB_CUR_MAX is 1, then we are just going to append.
    if (MB_CUR_MAX == 1) {
        printf("MB_CUR_MAX == 1\n");
        size_t in_pos = 0;
        while (in_pos < in_len) {
            result.push_back(static_cast<unsigned char>(in[in_pos]));
            in_pos++;
        }
        return result;
    }

    size_t in_pos = 0;
    mbstate_t state = {};
    while (in_pos < in_len) {
        printf("while start \n");
        // Append any initial sequence of ascii characters.
        // Note we do not support character sets which are not supersets of ASCII.
        size_t ascii_prefix_length = count_ascii_prefix(&in[in_pos], in_len - in_pos);
        result.insert(result.end(), &in[in_pos], &in[in_pos + ascii_prefix_length]);
        in_pos += ascii_prefix_length;
        assert(in_pos <= in_len && "Position overflowed length");
        if (in_pos == in_len) break;

        // We have found a non-ASCII character.
        bool use_encode_direct = false;
        size_t ret = 0;
        wchar_t wc = 0;

        if (false) {
#if defined(HAVE_BROKEN_MBRTOWC_UTF8)
        } else if ((in[in_pos] & 0xF8) == 0xF8) {
            // Protect against broken std::mbrtowc() implementations which attempt to encode UTF-8
            // sequences longer than four bytes (e.g., OS X Snow Leopard).
            use_encode_direct = true;
            printf("a\n");
#endif
        } else if (sizeof(wchar_t) == 2 &&  //!OCLINT(constant if expression)
                   (in[in_pos] & 0xF8) == 0xF0) {
            // Assume we are in a UTF-16 environment (e.g., Cygwin) using a UTF-8 encoding.
            // The bits set check will be true for a four byte UTF-8 sequence that requires
            // two UTF-16 chars. Something that doesn't work with our simple use of std::mbrtowc().
            use_encode_direct = true;
            printf("b\n");
        } else {
            ret = std::mbrtowc(&wc, &in[in_pos], in_len - in_pos, &state);
            // Determine whether to encode this character with our crazy scheme.
            if (wc >= ENCODE_DIRECT_BASE && wc < ENCODE_DIRECT_BASE + 256) {
                use_encode_direct = true;
                printf("c\n");
            } else if (wc == INTERNAL_SEPARATOR) {
                use_encode_direct = true;
                printf("d\n");
            } else if (ret == static_cast<size_t>(-2)) {
                // Incomplete sequence.
                use_encode_direct = true;
                printf("e\n");
            } else if (ret == static_cast<size_t>(-1)) {
                // Invalid data.
                use_encode_direct = true;
                printf("f\n");
            } else if (ret > in_len - in_pos) {
                // Other error codes? Terrifying, should never happen.
                use_encode_direct = true;
                printf("g\n");
            } else if (sizeof(wchar_t) == 2 && wc >= 0xD800 &&  //!OCLINT(constant if expression)
                       wc <= 0xDFFF) {
                // If we get a surrogate pair char on a UTF-16 system (e.g., Cygwin) then
                // it's guaranteed the UTF-8 decoding is wrong so use direct encoding.
                use_encode_direct = true;
                printf("h\n");
            }
        }

        if (use_encode_direct) {
            wc = ENCODE_DIRECT_BASE + static_cast<unsigned char>(in[in_pos]);
            result.push_back(wc);
            in_pos++;
            std::memset(&state, 0, sizeof state);
            printf("i\n");
        } else if (ret == 0) {  // embedded null byte!
            result.push_back(L'\0');
            in_pos++;
            std::memset(&state, 0, sizeof state);
            printf("j\n");
        } else {  // normal case
            result.push_back(wc);
            in_pos += ret;
            printf("k\n");
        }
    }

    return result;
}

void wcs2string_bad_char(wchar_t wc) {
    wprintf(L"char-encoding", L"Wide character U+%4X has no narrow representation", wc);
}

/// Implementation of wcs2string that accepts a callback.
/// This invokes \p func with (const char*, size_t) pairs.
/// If \p func returns false, it stops; otherwise it continues.
/// \return false if the callback returned false, otherwise true.
template <typename Func>
bool wcs2string_callback(const wchar_t *input, size_t len, const Func &func) {
    mbstate_t state = {};
    char converted[MB_LEN_MAX];

    for (size_t i = 0; i < len; i++) {
        wchar_t wc = input[i];
        // TODO: this doesn't seem sound.
        if (wc == INTERNAL_SEPARATOR) {
            // do nothing
        } else if (wc >= ENCODE_DIRECT_BASE && wc < ENCODE_DIRECT_BASE + 256) {
            converted[0] = wc - ENCODE_DIRECT_BASE;
            if (!func(converted, 1)) return false;
        } else if (MB_CUR_MAX == 1) {  // single-byte locale (C/POSIX/ISO-8859)
            // If `wc` contains a wide character we emit a question-mark.
            if (wc & ~0xFF) {
                wc = '?';
            }
            converted[0] = wc;
            if (!func(converted, 1)) return false;
        } else {
            std::memset(converted, 0, sizeof converted);
            size_t len = std::wcrtomb(converted, wc, &state);
            if (len == static_cast<size_t>(-1)) {
                wcs2string_bad_char(wc);
                std::memset(&state, 0, sizeof(state));
            } else {
                if (!func(converted, len)) return false;
            }
        }
    }
    return true;
}

void wcs2string_appending(const wchar_t *in, size_t len, std::string *receiver) {
    assert(receiver && "Null receiver");
    receiver->reserve(receiver->size() + len);
    wcs2string_callback(in, len, [&](const char *buff, size_t bufflen) {
        receiver->append(buff, bufflen);
        return true;
    });
}

std::string wcs2string(const wchar_t *in, size_t len) {
    if (len == 0) return std::string{};
    std::string result;
    wcs2string_appending(in, len, &result);
    return result;
}


std::wstring str2wcstring(const char *in) { return str2wcs_internal(in, std::strlen(in)); }

std::wstring str2wcstring(const std::string &in) {
    // Handles embedded nulls!
    return str2wcs_internal(in.data(), in.size());
}
