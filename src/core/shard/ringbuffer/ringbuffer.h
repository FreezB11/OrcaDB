///@file:ringbuffer.h
#pragma once
#include <stddef.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdalign.h>
#include "../proc.h"

typedef struct{
    alignas(64) _Atomic size_t hp; // writer
    alignas(64) _Atomic size_t rp; // reader
    size_t cap;
    proc_t** proc;
}rBuff; // ring buffer

void rBuff_init(rBuff* r, size_t cap);
// below is the skeleton
bool rBuff_write(rBuff* r, proc_t* p);
proc_t* rBuff_read(rBuff* r);
bool isFull(rBuff* r);

/*
Rule of thumb (important)

If two threads write to two different variables, those variables must not share a cache line.

Your ring buffer is a textbook case.
*/