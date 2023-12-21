#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compress a string into a buffer
 *
 * no more than n characters (including null terminator)
 * will be written into dst
 *
 * @param dst location to store compressed string
 * @param n size of dst
 * @param src text to compress
 */
void Dictionary_compress(char* dst, uint8_t n, char const* src);

#ifdef __cplusplus
} // extern "C" {
#endif