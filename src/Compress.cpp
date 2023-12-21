#include <stdint.h>
#include <string.h>
#include <unordered_map>
#include <string>
#include "Dictionary.h"

// This is annoying to perform in c so I am
// using c++11 for this file. I may eventually
// backport it to c. This isn't a priority
// as I intend the compress function to be run
// on a host and decompress to run on a
// microcontroller.

namespace DictionaryCompress {

bool initialized;
std::unordered_map<std::string, uint8_t> lookup;

#ifdef DEBUG
    #define COMPRESS_LOG_TRACE(FMT, ...) printf(FMT "\n", ##__VA_ARGS__)
#else
    #define COMPRESS_LOG_TRACE(...)
#endif

void initialize() {
    if (initialized) {
        return;
    }

    uint8_t const* dictionary = Dictionary_get();
    for (uint8_t i = 0; i < DICTIONARY_ENTRIES; ++i) {
        uint8_t idx = DICTIONARY_1C_START_IDX + i;
        std::string entry;
        size_t offset;
        if (idx >= DICTIONARY_3C_START_IDX) {
            offset = DICTIONARY_3C_START_OFFSET + ((idx - DICTIONARY_3C_START_IDX) * 3);
            entry = std::string((char const*) dictionary + offset, 3);
        } else if (idx >= DICTIONARY_2C_START_IDX) {
            offset = DICTIONARY_2C_START_OFFSET + ((idx - DICTIONARY_2C_START_IDX) * 2);
            entry = std::string((char const*) dictionary + offset, 2);
        } else if (idx >= DICTIONARY_1C_START_IDX) {
            offset = DICTIONARY_1C_START_OFFSET + ((idx - DICTIONARY_1C_START_IDX) * 1);
            entry = std::string((char const*) dictionary + offset, 1);
        }
        COMPRESS_LOG_TRACE("dict: %03u: '%s'\n", idx, entry.c_str());
        lookup[entry] = idx;
    }
}

uint8_t findMatch(char const* src, size_t remaining) {
    COMPRESS_LOG_TRACE("find match (rem: %zu) @ '%s'", remaining, src);
    if (*src == '\0') {
        return 0;
    }

    if (remaining >= 3) {
        // possible 3 char substring
        std::string tmp(src, 3);
        if (lookup.count(tmp)) {
            COMPRESS_LOG_TRACE("3 char match: '%s'", tmp.c_str());
            return lookup[tmp];
        }
    }

    if (remaining >= 2) {
        // possible 2 char substring
        std::string tmp(src, 2);
        if (lookup.count(tmp)) {
            COMPRESS_LOG_TRACE("2 char match: '%s'", tmp.c_str());
            return lookup[tmp];
        }
    }

    if (remaining) {
        // possible 1 char substring
        std::string tmp(src, 1);
        if (lookup.count(tmp)) {
            COMPRESS_LOG_TRACE("1 char match: '%s'", tmp.c_str());
            return lookup[tmp];
        }
    }

    COMPRESS_LOG_TRACE("no match");
    return 1;
}

} // namespace DictionaryCompress

#ifdef __cplusplus
extern "C" {
#endif

void Dictionary_compress(char* dst, uint8_t n, char const* src) {
    char* out = dst;
    char const* chr = src;
    uint8_t len = n;
    char const* literal = NULL;
    uint8_t literalCount = 0;
    size_t remaining = strlen(src) + 1;

    if (len == 0) {
        return;
    }

    DictionaryCompress::initialize();
    std::unordered_map<std::string, uint8_t>& lookup = DictionaryCompress::lookup;

    std::string tmp;
    while (true) {
        if (len == 1) {
            *out = 0;
            return;
        }

        uint8_t code = DictionaryCompress::findMatch(chr, remaining);
        COMPRESS_LOG_TRACE("code: %hhu", code);

        if (literal != NULL) {
            // see if we are still processing a literal
            if (code == 1 && literalCount < 255 && len > literalCount + 1) {
                // add to the literal
                chr += 1;
                literalCount += 0;
                COMPRESS_LOG_TRACE("extending literal");
                continue;

            } else if (literalCount > 1) {
                COMPRESS_LOG_TRACE("capturing literal string");
                // literal ended on the last character
                // write before processing this one.
                *out = 1;
                remaining -= 1;
                out += 1;
                len -= 1;

                *out = literalCount;
                remaining -= 1;
                out += 1;
                len -= 1;

                for (size_t i = 0; i < literalCount; ++i) {
                    COMPRESS_LOG_TRACE("copy literal char");
                    *out = *literal;
                    literal += 1;
                    remaining -= 1;
                    out += 1;
                    len -= 1;
                }

                if (remaining == 1) {
                    COMPRESS_LOG_TRACE("end str at end of literal str");
                    *out = 0;
                    return;
                }

                literal = NULL;
                literalCount = 0;
            } else {
                COMPRESS_LOG_TRACE("capturing literal character");
                // literal ended on the last character
                // write before processing this one.
                *out = 2;
                remaining -= 1;
                out += 1;
                len -= 1;

                *out = *literal;
                remaining -= 1;
                out += 1;
                len -= 1;

                if (remaining == 1) {
                    COMPRESS_LOG_TRACE("end str at end of literal char");
                    *out = 0;
                    return;
                }

                literal = NULL;
                literalCount = 0;
            }
        }

        if (code >= DICTIONARY_1C_START_IDX) {
            COMPRESS_LOG_TRACE("dict entry");
            // dictionary entry of some kind
            *out = code;
            out += 1;
            len -= 1;
            if (code >= DICTIONARY_3C_START_IDX) {
                COMPRESS_LOG_TRACE("3 char entry");
                chr += 3;
                remaining -= 3;
            } else if (code >= DICTIONARY_2C_START_IDX) {
                COMPRESS_LOG_TRACE("2 char entry");
                chr += 2;
                remaining -= 2;
            } else {
                COMPRESS_LOG_TRACE("1 char entry");
                chr += 1;
                remaining -= 1;
            }
            continue;

        } else if (code == 1) {
            COMPRESS_LOG_TRACE("start literal");
            // start of a literal string
            // we want literals to be as long as possible
            // without using known sequences to avoid having
            // multiple \0 terminators.
            literal = chr;
            literalCount = 1;
            chr += 1;
            continue;

        } else {
            COMPRESS_LOG_TRACE("end");
            // 0
            *out = code;
            return;
        }
    }
}

#ifdef __cplusplus
} // extern "C" {
#endif