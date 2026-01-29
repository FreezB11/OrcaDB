///@file:proc.h
#pragma once
#include <stddef.h>
#include <stdatomic.h>

typedef enum{
    OP_PUT,
    OP_DEL,
    OP_GET
}OP_CODE;

typedef struct{
    OP_CODE op;
    const char* key;
    void* value;
    size_t val_len;
    int free_value;
    void **out_val;
    int* status;
    _Atomic int done;
}proc_t;