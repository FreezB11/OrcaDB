///@file: hash.c
#include <stdint.h>
#include <stddef.h>
#include "../kv_string.h"

uint64_t FNV_1a(const void *data, size_t len) {
    const unsigned char *p = data;
    uint64_t hash = 14695981039346656037ULL;

    for (size_t i = 0; i < len; i++) {
        hash ^= (uint64_t)p[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}


uint64_t djb2(kv_string* key){
    uint64_t hash = 5381;
    for(size_t i = 0; i < key->len; i++){
        hash = ((hash << 5) + hash) + (unsigned char)key->str[i]; // hash* 33 +c
    }
    return hash;
}

// taken from online 
//Simple xxHash64
#define XXH_PRIME64_1  0x9E3779B185EBCA87ULL
#define XXH_PRIME64_2  0xC2B2AE3D27D4EB4FULL
#define XXH_PRIME64_3  0x165667B19E3779F9ULL
#define XXH_PRIME64_4  0x85EBCA77C2B2AE63ULL
#define XXH_PRIME64_5  0x27D4EB2F165667C5ULL

static inline uint64_t xxh_rotl64(uint64_t x, int r) {
    return (x << r) | (x >> (64 - r));
}

uint64_t xxhash64(const char* key, size_t len) {
    uint64_t h64;
    const unsigned char* p = (const unsigned char*)key;
    const unsigned char* end = p + len;
    
    if (len >= 32) {
        const unsigned char* limit = end - 32;
        uint64_t v1 = XXH_PRIME64_1 + XXH_PRIME64_2;
        uint64_t v2 = XXH_PRIME64_2;
        uint64_t v3 = 0;
        uint64_t v4 = -XXH_PRIME64_1;
        
        do {
            v1 += *(uint64_t*)p * XXH_PRIME64_2;
            v1 = xxh_rotl64(v1, 31);
            v1 *= XXH_PRIME64_1;
            p += 8;
            
            v2 += *(uint64_t*)p * XXH_PRIME64_2;
            v2 = xxh_rotl64(v2, 31);
            v2 *= XXH_PRIME64_1;
            p += 8;
            
            v3 += *(uint64_t*)p * XXH_PRIME64_2;
            v3 = xxh_rotl64(v3, 31);
            v3 *= XXH_PRIME64_1;
            p += 8;
            
            v4 += *(uint64_t*)p * XXH_PRIME64_2;
            v4 = xxh_rotl64(v4, 31);
            v4 *= XXH_PRIME64_1;
            p += 8;
        } while (p <= limit);
        
        h64 = xxh_rotl64(v1, 1) + xxh_rotl64(v2, 7) + 
              xxh_rotl64(v3, 12) + xxh_rotl64(v4, 18);
        
        // Merge
        v1 *= XXH_PRIME64_2; v1 = xxh_rotl64(v1, 31); v1 *= XXH_PRIME64_1;
        h64 ^= v1; h64 = h64 * XXH_PRIME64_1 + XXH_PRIME64_4;
        
        v2 *= XXH_PRIME64_2; v2 = xxh_rotl64(v2, 31); v2 *= XXH_PRIME64_1;
        h64 ^= v2; h64 = h64 * XXH_PRIME64_1 + XXH_PRIME64_4;
        
        v3 *= XXH_PRIME64_2; v3 = xxh_rotl64(v3, 31); v3 *= XXH_PRIME64_1;
        h64 ^= v3; h64 = h64 * XXH_PRIME64_1 + XXH_PRIME64_4;
        
        v4 *= XXH_PRIME64_2; v4 = xxh_rotl64(v4, 31); v4 *= XXH_PRIME64_1;
        h64 ^= v4; h64 = h64 * XXH_PRIME64_1 + XXH_PRIME64_4;
    } else {
        h64 = XXH_PRIME64_5;
    }
    
    h64 += (uint64_t)len;
    
    // Process remaining bytes
    while (p + 8 <= end) {
        uint64_t k1 = *(uint64_t*)p;
        k1 *= XXH_PRIME64_2;
        k1 = xxh_rotl64(k1, 31);
        k1 *= XXH_PRIME64_1;
        h64 ^= k1;
        h64 = xxh_rotl64(h64, 27) * XXH_PRIME64_1 + XXH_PRIME64_4;
        p += 8;
    }
    
    if (p + 4 <= end) {
        h64 ^= (uint64_t)(*(uint32_t*)p) * XXH_PRIME64_1;
        h64 = xxh_rotl64(h64, 23) * XXH_PRIME64_2 + XXH_PRIME64_3;
        p += 4;
    }
    
    while (p < end) {
        h64 ^= (*p++) * XXH_PRIME64_5;
        h64 = xxh_rotl64(h64, 11) * XXH_PRIME64_1;
    }
    
    // Finalization
    h64 ^= h64 >> 33;
    h64 *= XXH_PRIME64_2;
    h64 ^= h64 >> 29;
    h64 *= XXH_PRIME64_3;
    h64 ^= h64 >> 32;
    
    return h64;
}
//wrapper for the kv_string
uint64_t x2hash(kv_string* key){
    return xxhash64(key->str, key->len);
}