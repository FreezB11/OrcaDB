///@file: storage.h
#pragma once
#include "../hashmap/hashmap.h"

int db_save(hashmap* hm, const char* filename);
hashmap* db_load(const char* filename);
int db_save_proc(hashmap* hm, const char* filename);