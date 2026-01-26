///@file:ringbuffer.c
#include "ringbuffer.h"
#include <stdlib.h>

void rBuff_init(rBuff* r, size_t cap){
    r->proc = malloc(cap * sizeof(void));
    atomic_store(&r->hp,0);
    atomic_store(&r->rp,0);
    r->cap = cap;
}

void rBuff_write(rBuff *r, proc_t *p){
    atomic_load(&r->hp);
    atomic_load(&r->rp);
    if(r->hp == r->rp) return;
    r->hp++;
    r->proc[r->hp] = p;
}
