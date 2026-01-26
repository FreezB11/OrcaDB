///@file:ringbuffer.h
#pragma once
#include <stddef.h>
#include <stdatomic.h>
#include <stdbool.h>
#include "../proc.h"

typedef struct{
    proc_t** proc;
    _Atomic int hp; // writer
    _Atomic int rp; // reader
    size_t cap;
}rBuff; // ring buffer

void rBuff_init(rBuff* r, size_t cap);
// below is the skeleton
void rBuff_write(rBuff* r, proc_t* p);
void rBuff_read();
bool isFull();