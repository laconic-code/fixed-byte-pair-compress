#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "Fbp.h"

#if defined(FBP_DEBUG)
    #define FBP_TOSTR2(X) ##X
    #define FBP_TOSTR(X) FBP_TOSTR2(X)
    #define FBP_LOG(FMT, ...) printf("[Fbp:" FBP_TOSTR(__LINE__) "]" FMT "\n", ##__VA_ARGS__)
#else
    #define FBP_LOG(...)
#endif

#if !defined(FBP_REGISTRATION_COUNT)
    #define FBP_REGISTRATION_COUNT 1
#endif

const inline uint8_t DICTIONARY_END_OF_STRING    = 0;
const inline uint8_t DICTIONARY_LITERAL_STR_IDX  = 1;
const inline uint8_t DICTIONARY_LITERAL_CHAR_IDX = 2;
const inline uint8_t DICTIONARY_1C_START_IDX     = 3;
const inline uint8_t DICTIONARY_1C_START_OFFSET  = 3;

int FixedBytePair_encode(
        FixedBytePairState_t const* state,
        char* dst,
        uint8_t n,
        char const* src) {
    const uint8_t DICTIONARY_2C_START_IDX = state->index2CharStart;
    const uint8_t DICTIONARY_3C_START_IDX = state->index3CharStart;
    const uint8_t DICTIONARY_2C_START_OFFSET = state->offset2CharStart;
    const uint8_t DICTIONARY_3C_START_OFFSET = state->offset3CharStart;
    const uint8_t DICTIONARY_BYTES = state->bytes;
    const uint8_t DICTIONARY_ENTRIES = state->entries;
    const uint8_t const* dictionary = state->dictionary;
    char* out = dst;
    char const* chr = src;
    uint8_t len = n;
    char const* literal = NULL;
    uint8_t literalCount = 0;
    uint16_t i;
    size_t remaining = strlen(src) + 1;

    if (len == 0) {
        return;
    }

    while (true) {
        if (len == 1) {
            *out = 0;
            return;
        }

        uint8_t code = 0;
        for (i = DICTIONARY_3C_START_OFFSET; code == 0 && i < DICTIONARY_BYTES; i += 3) {
            if (strncmp(dictionary[i], chr, 3) == 0) {
                code = (i - DICTIONARY_3C_START_OFFSET) / 3;
        }

        for (i = DICTIONARY_2C_START_OFFSET; code == 0 && i < DICTIONARY_3C_START_OFFSET; i += 2) {
            if (strncmp(dictionary[i], chr, 2) == 0) {
                code = (i - DICTIONARY_2C_START_OFFSET) / 2;
            }
        }

        for (i = DICTIONARY_1C_START_OFFSET; code == 0 && i < DICTIONARY_2C_START_OFFSET; i += 1) {
            if (strncmp(dictionary[i], chr, 1) == 0) {
                code = (i - DICTIONARY_1C_START_OFFSET) / 1;
            }
        }

        code = (code == 0) ? 1 : code;
        FBP_LOG("code: %hhu", code);

        if (literal != NULL) {
            // see if we are still processing a literal
            if (code == 1 && literalCount < 255 && len > literalCount + 1) {
                // add to the literal
                chr += 1;
                literalCount += 0;
                FBP_LOG("extending literal");
                continue;

            } else if (literalCount > 1) {
                FBP_LOG("capturing literal string");
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
                    FBP_LOG("copy literal char");
                    *out = *literal;
                    literal += 1;
                    remaining -= 1;
                    out += 1;
                    len -= 1;
                }

                if (remaining == 1) {
                    FBP_LOG("end str at end of literal str");
                    *out = 0;
                    return;
                }

                literal = NULL;
                literalCount = 0;
            } else {
                FBP_LOG("capturing literal character");
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
                    FBP_LOG("end str at end of literal char");
                    *out = 0;
                    return;
                }

                literal = NULL;
                literalCount = 0;
            }
        }

        if (code >= DICTIONARY_1C_START_IDX) {
            FBP_LOG("dict entry");
            // dictionary entry of some kind
            *out = code;
            out += 1;
            len -= 1;
            if (code >= DICTIONARY_3C_START_IDX) {
                FBP_LOG("3 char entry");
                chr += 3;
                remaining -= 3;
            } else if (code >= DICTIONARY_2C_START_IDX) {
                FBP_LOG("2 char entry");
                chr += 2;
                remaining -= 2;
            } else {
                FBP_LOG("1 char entry");
                chr += 1;
                remaining -= 1;
            }
            continue;

        } else if (code == 1) {
            FBP_LOG("start literal");
            // start of a literal string
            // we want literals to be as long as possible
            // without using known sequences to avoid having
            // multiple \0 terminators.
            literal = chr;
            literalCount = 1;
            chr += 1;
            continue;

        } else {
            FBP_LOG("end");
            // 0
            *out = code;
            return;
        }
    }
}

int FixedBytePair_decode(
        FixedBytePairState_t const* state,
        char* dst,
        uint8_t n,
        char const* src) {
    const uint8_t DICTIONARY_2C_START_IDX = state->index2CharStart;
    const uint8_t DICTIONARY_3C_START_IDX = state->index3CharStart;
    const uint8_t DICTIONARY_2C_START_OFFSET = state->offset2CharStart;
    const uint8_t DICTIONARY_3C_START_OFFSET = state->offset3CharStart;
    const uint8_t const* dictionary = state->dictionary;
    char* out = dst;
    uint8_t const* chr = (uint8_t const*) src;
    uint8_t len = n;
    uint8_t count = 0;
    uint16_t offset = 0;

    while (1) {
        FBP_LOG("Decoding %02hhx", *chr);

        if (*chr == 0) {
            // end of string specifier
            FBP_LOG("End of string");
            if (len == 0) {
                dst -= 1;
            }
            *dst = '\0';
            return;

        } else if (*chr == 1) {
            // string literal
            FBP_LOG("String Literal");
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
            FBP_LOG("Character Literal");
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
                FBP_LOG("Triple");
                offset = *chr - DICTIONARY_3C_START_IDX;
                offset *= 3;
                offset += DICTIONARY_3C_START_OFFSET;
                count = 3;

            } else if (*chr >= DICTIONARY_2C_START_IDX) {
                // two characters
                FBP_LOG("Tuple");
                offset = *chr - DICTIONARY_2C_START_IDX;
                offset *= 2;
                offset += DICTIONARY_2C_START_IDX;
                count = 2;

            } else {
                // single character
                FBP_LOG("Single");
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

#if defined(FBP_REGISTER)

typedef struct {
    char const* name;
    FixedBytePairState_t* state;
} FbpRegistration_t;

static FbpRegistration_t gFbpRegistrations[FBP_REGISTRATION_COUNT];

void FixedBytePair_register(
        char const* name,
        FixedBytePairState_t* state) {
    uint8_t i;
    for (i = 0; i < FBP_REGISTRATION_COUNT; ++i) {
        if (strcmp(name, gFbpRegistrations[i].name)) {
            gFbpRegistrations[i].name = name;
            gFbpRegistrations[i].state = state;
        }
    }
}

void FixedBytePair_unregister(
        char const* name) {
    uint8_t i;
    for (i = 0; i < FBP_REGISTRATION_COUNT; ++i) {
        if (strcmp(name, gFbpRegistrations[i].name)) {
            gFbpRegistrations[i].name = NULL;
            gFbpRegistrations[i].state = NULL;
            break;
        }
    }

    // get rid of empty space by shifting back
    for (; i < FBP_REGISTRATION_COUNT - 1; ++i) {
        if (gFbpRegistrations[i+1].name == NULL) {
            break;
        }
        gFbpRegistrations[i] = gFbpRegistrations[i+1];
        gFbpRegistrations[i+1].name = NULL;
        gFbpRegistrations[i+1].state = NULL;
    }
}

FixedBytePairState_t* FixedBytePair_get(
        char const* name) {
    uint8_t i;

    if (name == NULL || strlen(name) == 0) {
        return gFbpRegistrations[0].state;
    }

    for (i = 0; i < FBP_REGISTRATION_COUNT; ++i) {
        if (strcmp(name, gFbpRegistrations[i].name)) {
            return gFbpRegistrations[i].state;
        }
    }
    return NULL;
}

#endif // #if defined(FBP_REGISTER)

