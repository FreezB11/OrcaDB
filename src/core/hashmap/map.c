#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <linux/time.h>

#define ITERATIONS 7
#define WARMUP_RUNS 1

#define h_cap 1 << 20 //~ [1 mil keys]

uint64_t FNV_1a(const void *data, size_t len) {
    const unsigned char *p = data;
    uint64_t hash = 14695981039346656037ULL;

    for (size_t i = 0; i < len; i++) {
        hash ^= (uint64_t)p[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

uint64_t djb2(const void *data, size_t len){
    const unsigned char *bytes = data;
    uint64_t hash = 5381;
    for(size_t i = 0; i < len; i++){
        hash = ((hash << 5) + hash) + bytes[i]; // hash* 33 +c
    }
    return hash;
}

typedef struct h_node{
    const char* key;
    const char* val;
    size_t key_len;
    size_t val_len;
    uint64_t hash;
    struct h_node* nxt;
}h_node;

typedef struct{
    size_t cap;
    size_t cnt;
    float load_factor;
    h_node** buckets;
}hashmap;

hashmap* hm_init(size_t cap){
    hashmap *hm = malloc(sizeof(hashmap));
    hm->buckets = calloc(cap, sizeof(h_node*));
    hm->load_factor = 0.9;
    hm->cap = cap;
    hm->cnt = 0;
    return hm;
}


static size_t next_pow2(size_t x) {
    size_t p = 1;
    while (p < x) p <<= 1;
    return p;
}

static void hm_rehash(hashmap *hm, size_t new_cap) {
    h_node **new_buckets = calloc(new_cap, sizeof(h_node*));

    for (size_t i = 0; i < hm->cap; i++) {
        h_node *node = hm->buckets[i];
        while (node) {
            h_node *next = node->nxt;
            size_t idx = node->hash & (new_cap - 1);
            node->nxt = new_buckets[idx];
            new_buckets[idx] = node;
            node = next;
        }
    }

    free(hm->buckets);
    hm->buckets = new_buckets;
    hm->cap = new_cap;
}

static inline void hm_maybe_resize(hashmap *hm) {
    if ((float)hm->cnt / hm->cap >= hm->load_factor) {
        hm_rehash(hm, hm->cap << 1);
    }
}

void hm_insert(hashmap* hm, const char* key, size_t key_len,
               const char* val, size_t val_len)
{
    uint64_t h1 = FNV_1a(key, key_len);
    uint64_t h2 = djb2(key, key_len) | 1;
    uint64_t hash = h1 + h2;
    size_t idx = hash & (hm->cap - 1);
    h_node *node = hm->buckets[idx];
    while (node) {
        if (node->hash == hash &&
            node->key_len == key_len &&
            memcmp(node->key, key, key_len) == 0){
            free((void*)node->val);
            node->val = malloc(val_len);
            memcpy((void*)node->val, val, val_len);
            node->val_len = val_len;
            return;
        }
        node = node->nxt;
    }
    h_node *new_node = malloc(sizeof(h_node));
    new_node->key = malloc(key_len);
    new_node->val = malloc(val_len);
    memcpy((void*)new_node->key, key, key_len);
    memcpy((void*)new_node->val, val, val_len);
    new_node->key_len = key_len;
    new_node->val_len = val_len;
    new_node->hash = hash;
    new_node->nxt = hm->buckets[idx];
    hm->buckets[idx] = new_node;

    hm->cnt++;
    hm_maybe_resize(hm);
}


char* hm_get(hashmap* hm, const char* key, size_t key_len){
    if(!hm || !key) return NULL;
    uint64_t h1 = FNV_1a(key, key_len);
    uint64_t h2 = djb2(key, key_len) | 1;
    uint64_t hash = h1 + h2;
    size_t idx = hash & (hm->cap - 1);
    h_node *node = hm->buckets[idx];
    while(node){
        if(node->hash == hash &&
            node->key_len == key_len &&
            memcmp(node->key, key, key_len) == 0){
                return node->val;
            }
        node = node->nxt;
    }
    return NULL;
}

int hm_delete(hashmap *hm, const char *key, size_t key_len) {
    if (!hm || !key) return 0;

    uint64_t h1 = FNV_1a(key, key_len);
    uint64_t h2 = djb2(key, key_len) | 1;
    uint64_t hash = h1 + h2;
    size_t idx = hash & (hm->cap - 1);

    h_node *node = hm->buckets[idx];
    h_node *prev = NULL;

    while (node) {
        if (node->hash == hash &&
            node->key_len == key_len &&
            memcmp(node->key, key, key_len) == 0) {

            if (prev) prev->nxt = node->nxt;
            else hm->buckets[idx] = node->nxt;

            free((void*)node->key);
            free((void*)node->val);
            free(node);
            hm->cnt--;
            return 1;
        }
        prev = node;
        node = node->nxt;
    }
    return 0;
}

static inline uint64_t nsec_now() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// void run_benchmark(size_t cap, float fill_ratio) {
//     size_t target = (size_t)(cap * fill_ratio);

//     hashmap *hm = hm_init(cap);

//     char keybuf[64];
//     char valbuf[64];

//     // INSERT benchmark
//     uint64_t t0 = nsec_now();
//     for (size_t i = 0; i < target; i++) {
//         snprintf(keybuf, sizeof(keybuf), "key_%zu", i);
//         snprintf(valbuf, sizeof(valbuf), "val_%zu", i);
//         hm_insert(hm, keybuf, strlen(keybuf), valbuf, strlen(valbuf));
//     }
//     uint64_t t1 = nsec_now();

//     // LOOKUP benchmark (random hits)
//     uint64_t t2 = nsec_now();
//     for (size_t i = 0; i < target; i++) {
//         size_t r = rand() % target;
//         snprintf(keybuf, sizeof(keybuf), "key_%zu", r);
//         char *v = hm_get(hm, keybuf, strlen(keybuf));
//         if (!v) abort();
//     }
//     uint64_t t3 = nsec_now();

//     printf(
//         "Fill %.0f%% | items=%zu | cap=%zu | "
//         "insert=%.2f ns/op | lookup=%.2f ns/op\n",
//         fill_ratio * 100.0,
//         target,
//         hm->cap,
//         (double)(t1 - t0) / target,
//         (double)(t3 - t2) / target
//     );
// }
void run_benchmark(size_t cap, float fill_ratio) {
    size_t target = (size_t)(cap * fill_ratio);

    double insert_sum = 0.0;
    double lookup_sum = 0.0;
    int samples = 0;

    char keybuf[64];
    char valbuf[64];

    for (int it = 0; it < ITERATIONS; it++) {
        hashmap *hm = hm_init(cap);

        // ---------------- INSERT ----------------
        uint64_t t0 = nsec_now();
        for (size_t i = 0; i < target; i++) {
            snprintf(keybuf, sizeof(keybuf), "key_%zu", i);
            snprintf(valbuf, sizeof(valbuf), "val_%zu", i);
            hm_insert(hm, keybuf, strlen(keybuf),
                          valbuf, strlen(valbuf));
        }
        uint64_t t1 = nsec_now();

        // ---------------- LOOKUP ----------------
        uint64_t t2 = nsec_now();
        for (size_t i = 0; i < target; i++) {
            size_t r = rand() % target;
            snprintf(keybuf, sizeof(keybuf), "key_%zu", r);
            char *v = hm_get(hm, keybuf, strlen(keybuf));
            if (!v) abort();
        }
        uint64_t t3 = nsec_now();

        double insert_ns = (double)(t1 - t0) / target;
        double lookup_ns = (double)(t3 - t2) / target;

        // skip warmup runs
        if (it >= WARMUP_RUNS) {
            insert_sum += insert_ns;
            lookup_sum += lookup_ns;
            samples++;
        }

        // NOTE: intentionally leaking hm here for benchmark purity
        // freeing would add noise; OS will reclaim on exit
    }

    printf(
        "Fill %3.0f%% | items=%zu | "
        "insert(avg)=%.2f ns/op | lookup(avg)=%.2f ns/op | runs=%d\n",
        fill_ratio * 100.0,
        target,
        insert_sum / samples,
        lookup_sum / samples,
        samples
    );
}

int main() {
    srand(123);

    size_t base_cap = 1 << 21;
    float levels[] = {0.10, 0.30, 0.50, 0.75, 0.90};

    for (size_t i = 0; i < sizeof(levels)/sizeof(levels[0]); i++) {
        run_benchmark(base_cap, levels[i]);
    }
    return 0;
}
