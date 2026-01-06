#pragma once
#include <stddef.h>
#include <stdint.h>

/* Opaque types */
typedef struct hm_node hm_node;
typedef struct map hashmap;

/**
 * @brief Create a hashmap
 * @param capacity Number of buckets
 * @return Pointer to hashmap on success, NULL on failure
 */
hashmap* hm_create(size_t capacity);

/**
 * @brief Insert or update a key-value pair
 * @param hm    Hashmap instance
 * @param key   Null-terminated key string
 * @param value - ptr to value(caller owns the memory)
 * @param val_len -size of value in bytes (for metadata, not used for copy)
 * @return 0 on success, -1 on allocation failure
 */
int hm_insert(hashmap* hm, const char* key, void* value, size_t val_len);

/**
 * @brief Retrieve value for a given key
 * @param hm  Hashmap instance
 * @param key Null-terminated key string
 * @return Pointer to value string, or NULL if not found
 * @note Returned pointer is owned by the hashmap. Do not free it.
 */
void* hm_get(hashmap* hm, const char* key);

/**
 * @brief Delete a key-value pair from hashmap
 * @param hm  Hashmap instance
 * @param key Null-terminated key string
 * @param free_value - if true, calls free() on the value pointer
 * @return 0 on success, -1 if key not found
 */
int hm_delete(hashmap* hm, const char* key, int free_value);

/**
 * @brief Destroy hashmap and free all associated memory
 * @param hm Hashmap instance
 * @param free_values - if true, calls free() on all value pointers
 */
void hm_free(hashmap* hm, int free_values);
