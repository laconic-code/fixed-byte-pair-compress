#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Decompress a string into a buffer
 *
 * no more than n characters (including null terminator)
 * will be written to dst.
 *
 * @param dst location to store expanded string
 * @param n size of dst
 * @param src compressed text to expand
 */
void Dictionary_decompress(char* dst, uint8_t n, char const* src);

#ifdef __cplusplus
} // extern "C" {
#endif
