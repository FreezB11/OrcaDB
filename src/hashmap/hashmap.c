///@file: hashmap.c
// all we require is string to string map
// **update** we need the value to be of void* coz yeah
// we need them to lets say store pointers
// so the input string must be hashed and maped
// internaly form a key-value pair and we will
// have a limit the is the capacity 
// lets take it to be 1024
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
// we will need a hash function
#include "../hash/hash.h"
#include "hashmap.h"


static inline size_t idx(hashmap* hm, uint64_t hash){
    return hash & (hm->capacity - 1);
}


/// @struct hashmap
/// @brief creates hashmap
hashmap* hm_create(size_t capacity){
    hashmap* hm = calloc(1, sizeof(hashmap));
    if(!hm) return NULL;
    hm->buckets = calloc(capacity, sizeof(hm_node));
    if(!hm->buckets){
        free(hm); return NULL;
    }
    hm->capacity = capacity;
    hm->count = 0;
    hm->load_factor = 0.75; // resize at 75% filled

    return hm;
}

/// @brief insert or update kv-pair
/// @param value - ptr to value(caller owns the memory)
/// @param val_len - size of value in bytes ( for metadata, not used for copy)
/// @return 0 on success, -1 on error
int hm_insert(hashmap* hm, const char* key, void* value, size_t val_len){
    if(!hm || !key || !key) return -1;
    size_t key_len = strlen(key);
    // we will calculate the hash
    // since we aint bothering with hashing 
    // just got the code from online and added to hash.h
    uint64_t hash = FNV_1a(key, key_len);

    hm_node cur={
        .hash = hash,
        .key = key,
        .value = value,
        .key_len = key_len,
        .val_len = val_len,
        .dib = 0,
        .flags = 0
    };

    // now we bother with the bucket index
    // from the computed hash value
    size_t bucket_idx = idx(hm, hash);
    // we check if the key exist
    // that is a collision so will update the value
    for(;;){
        hm_node* node = &hm->buckets[bucket_idx];
        if(node->hash == 0 || (node->flags & HM_FLAG_TOMB)){
            *node = cur;
            hm->count++;
            return 0;
        }
        if(node->hash == hash && 
            node->key_len == key_len &&
            memcmp(node->key, key, key_len) == 0){
                // key exist, update the value
                // free((void*)node->value); // since caller manages the memory now
                // node->value = strdup(value);
                node->value = value;
                node->val_len = val_len;
                return 0;
            }
        // node = node->next;
        if(node->dib < cur.dib){
            hm_node tmp = *node;
            *node = cur;
            cur = tmp;
        }
        cur.dib++;
        bucket_idx = (bucket_idx + 1) & (hm->capacity - 1);
    }

    // new key , so we create node
    // hm_node* new_node = malloc(sizeof(hm_node));
    // if(!new_node) return -1;

    // new_node->key = strdup(key);
    // if(!new_node->key){
    //     free(new_node);
    //     return -1;
    // }
    // new_node->key_len = key_len;
    // // new_node->value = strdup(value);
    // new_node->value = value;
    // new_node->val_len = val_len;
    // new_node->hash = hash;

    // //insert at head of bucket
    // new_node->next = hm->buckets[bucket_idx];
    // hm->buckets[bucket_idx] = new_node;
    // hm->count++;

    ///@todo: check the load factor and then we resize
    return 0;
}

/// @brief get value for key
/// @return pointer to the value string, or null if not found
void* hm_get(hashmap* hm, const char* key){
    if(!hm || !key) return NULL;

    size_t key_len = strlen(key);
    uint64_t hash = FNV_1a(key, key_len);
    size_t bucket_idx = idx(hm, hash);
    uint16_t dib = 0;
    for(;;){
        hm_node* node = &hm->buckets[bucket_idx];
        if(node->hash == 0 || dib > node->dib) return NULL;
        if(node->hash == hash &&
            node->key_len == key_len &&
            memcmp(node->key, key, key_len) == 0){
                return node->value;
            }
        // node = node->next;
        dib++;
        bucket_idx = (bucket_idx + 1) & (hm->capacity - 1);
    }
    return NULL;
}

/// @brief delete key from hashmap
/// @param free_value - if true, calls free() on the value pointer
/// @return 0 on success, -1 if not found
int hm_delete(hashmap* hm, const char*key, int free_value){
    if(!hm || !key) return -1;

    size_t key_len  = strlen(key);
    uint64_t hash = FNV_1a(key, key_len);
    size_t bucket_idx = idx(hm, hash);
    uint16_t dib = 0;
    // hm_node* prev = NULL;

    for(;;){
        hm_node* node = &hm->buckets[bucket_idx];
        if(node->hash == 0 || dib > node->dib) return -1;
        if(node->hash == hash &&
            node->key_len == key_len &&
            memcmp(node->key, key, key_len) == 0){
                node->flags |= HM_FLAG_TOMB;
                hm->count--;
                return 0;
            }
        // prev = node;
        // node = node->next;
        dib++;
        bucket_idx = (bucket_idx + 1) & (hm->capacity - 1);
    }
    return -1;
}

///@brief destroy/clean
///@param free_values - if true, calls free() on all value pointers
void hm_free(hashmap* hm, int free_values){
    if(!hm) return;

    // for(size_t i = 0; i < hm->capacity; i++){
    //     hm_node* node = hm->buckets[i];
    //     while(node){
    //         hm_node* next = node->next;
    //         free((void*)node->key);
    //         // free((void*)node->value);
    //         if(free_values && node->value){
    //             free(node->value);
    //         }
    //         free(node);
    //         node=next;
    //     }
    // }
    free(hm->buckets);
    free(hm);
}