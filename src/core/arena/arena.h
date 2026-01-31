///@file: arena.h
#pragma once
// finally the part i was waiting for
#include <stddef.h>
#include <stdint.h>

typedef struct arena{
    uint8_t* base;
    size_t size;
    size_t offset;
}arena;

arena* arena_create(size_t size);
void* arena_alloc(arena* a, size_t size);
void arena_reset(arena* a);
void arena_free(arena* a);