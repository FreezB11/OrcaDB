///@file: util.h
#pragma once
#include <stddef.h>
#include <stdint.h>

bool file_exists(const char *filename);

/*
    what i want is this to generate the file name but i have 
    given the func name as id_generate but yea
    we must have a random value then the shard_id and replication_id
    this will be the file name
*/
uint64_t generateID();