///@file: aof.h
#pragma once
#include <stdio.h>
#include <time.h>
#include "../hashmap/hashmap.h"
 
typedef enum{
    AOF_SYNC_ALWAYS, // fsync after every write
    AOF_SYNC_EVERYSEC, // fsync every second
    AOF_SYNC_NO //let os decide
}aof_sync_policy;

// AOF handle
typedef struct{
    FILE* file;
    char* filename;
    aof_sync_policy sync_policy;
    time_t last_sync;
    size_t bytes_written;
}aof_t;

aof_t* aof_open(const char* filename, aof_sync_policy policy);
void aof_close(aof_t* aof);
int aof_append_set(aof_t* aof, const char* key, const void* value, size_t val_len);
int aof_append_del(aof_t* aof, const char* key);
int aof_replay(aof_t* aof, hashmap* hm);
int aof_rewrite(aof_t* aof, hashmap* hm);