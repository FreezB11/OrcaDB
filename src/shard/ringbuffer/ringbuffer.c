///@file:ringbuffer.c
#include "ringbuffer.h"
#include <stdlib.h>

void rBuff_init(rBuff* r, size_t cap){
    r->proc = malloc(cap * sizeof(proc_t *));
    atomic_store(&r->hp,0);
    atomic_store(&r->rp,0);
    r->cap = cap;
}

bool rBuff_write(rBuff *r, proc_t *p){
    size_t hp = atomic_load_explicit(&r->hp, memory_order_relaxed);
    size_t rp = atomic_load_explicit(&r->rp, memory_order_acquire);
    // this is my initial buggy code
    // if(r->hp == r->rp) return;
    // // we are assuming the cap to be power of two
    // int idx = (r->hp) & (r->cap - 1);
    // r->proc[idx] = p;
    // r->hp = idx++;
    if(((hp+1) & (r->cap - 1)) == rp) return false;
    r->proc[hp & (r->cap - 1)] = p;
    atomic_store_explicit(&r->hp, hp+1, memory_order_release);
    return true;
}
proc_t* rBuff_read(rBuff *r){
    size_t hp = atomic_load_explicit(&r->hp, memory_order_relaxed);
    size_t rp = atomic_load_explicit(&r->rp, memory_order_acquire);
    if(rp == hp) return NULL;

    proc_t *p = r->proc[rp & (r->cap - 1)];
    atomic_store_explicit(&r->rp, rp+1, memory_order_release);
    return p;
}

bool isFull(rBuff *r){
    size_t hp = atomic_load_explicit(&r->hp, memory_order_relaxed);
    size_t rp = atomic_load_explicit(&r->rp, memory_order_acquire);
    return ((hp+1) & (r->cap - 1)) == rp;
}
