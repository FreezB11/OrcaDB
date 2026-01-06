#include "lru_cache.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define TABLE_SIZE 1024  // internal hash table size

typedef struct lru_node {
    char* key;
    char* value;
    struct lru_node* prev;
    struct lru_node* next;
    struct lru_node* hnext; // hash-chain next
} lru_node;

struct lru_cache {
    size_t capacity;
    size_t count;
    lru_node* head;
    lru_node* tail;
    lru_node** table;
};

/* ---------------- Hash ---------------- */

static uint64_t hash_key(const char* s) {
    uint64_t h = 1469598103934665603ULL;
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 1099511628211ULL;
    }
    return h;
}

static lru_node* ht_get(lru_cache* c, const char* key) {
    size_t idx = hash_key(key) % TABLE_SIZE;
    lru_node* n = c->table[idx];
    while (n) {
        if (strcmp(n->key, key) == 0)
            return n;
        n = n->hnext;
    }
    return NULL;
}

static void ht_insert(lru_cache* c, lru_node* node) {
    size_t idx = hash_key(node->key) % TABLE_SIZE;
    node->hnext = c->table[idx];
    c->table[idx] = node;
}

static void ht_remove(lru_cache* c, const char* key) {
    size_t idx = hash_key(key) % TABLE_SIZE;
    lru_node* cur = c->table[idx];
    lru_node* prev = NULL;

    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            if (prev) prev->hnext = cur->hnext;
            else c->table[idx] = cur->hnext;
            return;
        }
        prev = cur;
        cur = cur->hnext;
    }
}

/* ---------------- LRU list ---------------- */

static void move_to_head(lru_cache* c, lru_node* n) {
    if (n == c->head) return;

    if (n->prev) n->prev->next = n->next;
    if (n->next) n->next->prev = n->prev;
    if (n == c->tail) c->tail = n->prev;

    n->prev = NULL;
    n->next = c->head;
    if (c->head) c->head->prev = n;
    c->head = n;
    if (!c->tail) c->tail = n;
}

static void evict_tail(lru_cache* c) {
    if (!c->tail) return;

    lru_node* n = c->tail;
    ht_remove(c, n->key);

    if (n->prev) n->prev->next = NULL;
    c->tail = n->prev;

    if (n == c->head) c->head = NULL;

    free(n->key);
    free(n->value);
    free(n);
    c->count--;
}

/* ---------------- API ---------------- */

lru_cache* lru_create(size_t capacity) {
    lru_cache* c = calloc(1, sizeof(lru_cache));
    c->capacity = capacity;
    c->table = calloc(TABLE_SIZE, sizeof(lru_node*));
    return c;
}

void lru_put(lru_cache* c, const char* key, const char* value) {
    lru_node* n = ht_get(c, key);

    if (n) {
        free(n->value);
        n->value = strdup(value);
        move_to_head(c, n);
        return;
    }

    if (c->count == c->capacity)
        evict_tail(c);

    n = calloc(1, sizeof(lru_node));
    n->key = strdup(key);
    n->value = strdup(value);

    n->next = c->head;
    if (c->head) c->head->prev = n;
    c->head = n;
    if (!c->tail) c->tail = n;

    ht_insert(c, n);
    c->count++;
}

const char* lru_get(lru_cache* c, const char* key) {
    lru_node* n = ht_get(c, key);
    if (!n) return NULL;
    move_to_head(c, n);
    return n->value;
}

void lru_free(lru_cache* c) {
    lru_node* n = c->head;
    while (n) {
        lru_node* next = n->next;
        free(n->key);
        free(n->value);
        free(n);
        n = next;
    }
    free(c->table);
    free(c);
}
