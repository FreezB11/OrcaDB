///@file:arena.c
#include "arena.h"
#include <stdlib.h>
#include <string.h>

arena* arena_create(size_t size){
    arena* a = malloc(sizeof(arena));
    if(!a) return NULL;
    
    a->base = aligned_alloc(64, size);
    if(!a->base){
        free(a);
        return NULL;
    }
    a->size = size;
    a->offset = 0;
    return a;
}

void* arena_alloc(arena* a, size_t size){
    size = (size + 7) & ~7UL; // 8 - byte alignment
    if(a->offset + size > a->size) return NULL;
    void *ptr = a->base + a->offset;
    a->offset += size;
    return ptr;
}

void arena_reset(arena* a){
    a->offset = 0;
}

void arena_free(arena* a){
    free(a->base);
    free(a);
}