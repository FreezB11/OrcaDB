///@file:shard.h
#include <stddef.h>

void* shard_worker(void *arg);
void shard_init();
void db_start();
void db_insert(const char* key, void* value, size_t val_len);