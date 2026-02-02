#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

void hm_insert(hashmap* hm, const char* key, size_t key_len,
               const char* val, size_t val_len)
{
    uint64_t hash = FNV_1a(key, key_len);
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
}


char* hm_get(hashmap* hm, const char* key, size_t key_len){
    if(!hm || !key) return NULL;
    uint64_t hash = FNV_1a(key, key_len);
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

int main(){
    char *test_key = "hello";
    char *test_val = "world";
    int key_len = strlen(test_key);
    int val_len = strlen(test_val);
    hashmap *test = hm_init(16);
    hm_insert(test, test_key, key_len, test_val, val_len);

    char *validate = hm_get(test, test_key, key_len);
    if(memcmp(test_val, validate, val_len) == 0){
        printf("we got it right\n");
    }
    printf("%s\n", validate);
    return 0;
}