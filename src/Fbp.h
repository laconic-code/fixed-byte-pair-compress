#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t index2CharStart;
    uint8_t index3CharStart;
    uint8_t offset2CharStart;
    uint8_t offset3CharStart;
    uint8_t entries;
    uint8_t bytes;
    uint8_t* dictionary;
} FixedBytePairState_t;

/**
 * Compress a string into a buffer
 *
 * no more than n characters (including null terminator)
 * will be written into dst
 *
 * @param dst location to store compressed string
 * @param n size of dst
 * @param src text to compress
 * @return number of bytes written (or that would be written)
 *         to dst (including terminating \0). if the returned
 *         value is more than n, the result will have been
 *         truncated w/ a terminating \0 (if possible). a
 *         negative value indicates an error.
 */
int FixedBytePair_encode(
    FixedBytePairState_t const* state,
    uint8_t* dst,
    int n,
    char const* src);

/**
 * Decompress a string into a buffer
 *
 * no more than n characters (including null terminator)
 * will be written to dst.
 *
 * @param dst location to store expanded string
 * @param n size of dst
 * @param src compressed text to expand
 * @return number of bytes written (or that would be written)
 *         to dst (including terminating \0). if the returned
 *         value is more than n, the result will have been
 *         truncated w/ a terminating \0 (if possible). a
 *         negative value indicates an error.
 */
int FixedBytePair_decode(
    FixedBytePairState_t const* state,
    char* dst,
    int n,
    uint8_t const* src);

#if defined(FBP_REGISTER)

/**
 * Add a compression algorithm state to the known states
 * 
 * does not print anything on error
 *
 * @param name string name of the compression algorithm
 * @param state compression algorithm constants
 */
void FixedBytePair_register(
    char const* name,
    FixedBytePairState_t* state);

/**
 * Remove a compression algorithm state to the known states
 *
 * @param name string name of the compression algorithm
 */
void FixedBytePair_unregister(
    char const* name);

/**
 * Get a compression algorithm state by name
 * 
 * @param name string name of the compression algorithm
 * @return compression algorithm state or NULL
 */
FixedBytePairState_t* FixedBytePair_get(
    char const* name);

#endif // #if defined(FBP_REGISTER)

#ifdef __cplusplus
} // extern "C" {
#endif
