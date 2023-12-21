#include <stdbool.h>
#include "Decompress.h"
#include "Dictionary.h"

#ifdef DEBUG
    #define DECOMPRESS_LOG_TRACE(FMT, ...) printf(FMT "\n", ##__VA_ARGS__)
#else
    #define DECOMPRESS_LOG_TRACE(...)
#endif

#ifdef __cplusplus
extern "C" {
#endif

void Dictionary_decompress(char* dst, uint8_t n, char const* src) {
    uint8_t const* dictionary = Dictionary_get();
    char* out = dst;
    uint8_t const* chr = (uint8_t const*) src;
    uint8_t len = n;
    uint8_t count = 0;
    uint16_t offset = 0;

    while (true) {
        DECOMPRESS_LOG_TRACE("Decoding %02hhx", *chr);

        if (*chr == 0) {
            // end of string specifier
            DECOMPRESS_LOG_TRACE("End of string");
            if (len == 0) {
                dst -= 1;
            }
            *dst = '\0';
            return;

        } else if (*chr == 1) {
            // string literal
            DECOMPRESS_LOG_TRACE("String Literal");
            chr += 1;
            count = *chr;

            chr += 1;
            while (count) {
                if (len == 1) {
                    *dst = '\0';
                    return;
                }
                *dst = *chr;
                dst += 1;
                len -= 1;
                count -= 1;
                chr += 1;
            }

        } else if (*chr == 2) {
            // char literal
            DECOMPRESS_LOG_TRACE("Character Literal");
            chr += 1;

            if (len == 1) {
                *dst = '\0';
                return;
            }

            // copy single char
            *dst = *chr;
            dst += 1;
            len -= 1;
            count -= 1;
            chr += 1;

        } else {
            // dictionary entry

            // the price we pay for not storing '\0' in each
            // entry of the dictionary is to have to figure
            // out the index based on the varying size
            // of previous table sections.

            count = 0;
            if (*chr >= DICTIONARY_3C_START_IDX) {
                // three characters
                DECOMPRESS_LOG_TRACE("Triple");
                offset = *chr - DICTIONARY_3C_START_IDX;
                offset *= 3;
                offset += DICTIONARY_3C_START_OFFSET;
                count = 3;

            } else if (*chr >= DICTIONARY_2C_START_IDX) {
                // two characters
                DECOMPRESS_LOG_TRACE("Tuple");
                offset = *chr - DICTIONARY_2C_START_IDX;
                offset *= 2;
                offset += DICTIONARY_2C_START_IDX;
                count = 2;

            } else {
                // single character
                DECOMPRESS_LOG_TRACE("Single");
                offset = *chr;
                count = 1;
            }

            while (count) {
                if (len == 1) {
                    *dst = '\0';
                    return;
                }
                #if defined(ARDUINO_ARCH_AVR)
                *dst = pgm_read_byte(&(dictionary[offset]));
                #else
                *dst = dictionary[offset];
                #endif
                dst += 1;
                len -= 1;
                count -= 1;
                offset += 1;
            }
            chr += 1;
        }
    }
}

#ifdef __cplusplus
} // extern "C" {
#endif