#pragma once
#include <stddef.h>
#include <stdint.h>

#define HM_CAP 4096 // hashmap-capacity

typedef struct hm_node
{
    // 12bytes but it will be padded to 16 i belive
    const char* key;
    // same goes here so it will be 32bytes till here
    void* value;
    // pointer so 8 bytes [40bytes till now]
    struct hm_node* next;
    // struct hm_node* prev;
    // this is also 8 bytes [48 bytes so far] this is okay ig
    uint64_t hash; // we can save the hash to save recomputation

    // length stored explicitly
    size_t key_len;
    size_t val_len;
}hm_node; // 48 bytes;

// we will stack align this; i.e. we try to keep the size as
// small as possible by arranging the components correctly
typedef struct hashmap{
    // 8bytes
    hm_node** buckets; // array of bucket heads;
    // size_t is 8 bytes [16bytes] so far
    size_t capacity;
    // this too is 8 bytes [24 bytes]
    size_t count;
    // 4 bytes that will [28 bytes]
    float load_factor; // trig to resize when threshold hit;
    // this will be padded and become 32 bytes;
}hashmap;
/*
    we will do a analysis
    hashmap == 32 bytes
    hm_node** bucket since we will have 4096 entries
    4096*8 = 32,768 bytes = 32 KB
    memory for nodes 
    4096*48 = 196,608 bytes = 192 KB

    total = 229,408bytes = 224KB = 0.22MB
    this is a rough figure while the real value is more than this
*/

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
