///@file: orca.c
#include "orca.h"
#include <stdio.h>
#include "./shard/shard.h"

Orca *_internal;

void _insert(const char* key, void *val){

}

void* _get(const char* key){

}

/*
    we will want one Aof and N x <id>.rdb file per shard 
*/
Orca* orca_init(){
    printf("[Orca] db initialized....\n");
    /*
        @todo        
        fix this work flow please !!!!!
    */
    db_start();

    _internal->get = _get;
    _internal->insert = _insert;

    return _internal;
}