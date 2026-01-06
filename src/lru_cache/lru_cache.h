#pragma once
#include <stddef.h>

typedef struct lru_cache lru_cache;

lru_cache* lru_create(size_t capacity);
void       lru_put(lru_cache* c, const char* key, const char* value);
const char* lru_get(lru_cache* c, const char* key);
void       lru_free(lru_cache* c);
