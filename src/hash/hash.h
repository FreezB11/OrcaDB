#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * @file hash.h
 * @brief Hash function interfaces for kv_string and raw byte buffers.
 *
 * This header exposes multiple non-cryptographic hash functions intended
 * for use in hash tables, key-value stores, and similar data structures.
 * All hashes return 64-bit values.
 */

/**
 * @brief Compute the 64-bit FNV-1a hash of a kv_string.
 *
 * FNV-1a is a fast, simple hash with good dispersion for small keys.
 *
 * @param key Pointer to a kv_string containing the input data
 * @return 64-bit hash value
 */
uint64_t FNV_1a(const char* key, size_t len);

/**
 * @brief Compute the djb2 hash of a kv_string.
 *
 * djb2 is a classic hash algorithm using a multiply-by-33 scheme.
 *
 * @param key Pointer to a kv_string containing the input data
 * @return 64-bit hash value
 */
uint64_t djb2(const char* key);

/**
 * @brief Compute the 64-bit xxHash of a raw byte buffer.
 *
 * xxHash is a very fast, non-cryptographic hash algorithm with excellent
 * avalanche properties. This implementation corresponds to xxHash64.
 *
 * @param key Pointer to the input byte buffer
 * @param len Length of the buffer in bytes
 * @return 64-bit hash value
 */
uint64_t xxhash64(const char* key, size_t len);