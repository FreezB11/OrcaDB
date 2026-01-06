#include "../src/hashmap/hashmap.h"
#include "../src/storage/aof.h"
#include "../src/storage/storage.h"

int main() {
    hashmap* db = NULL;
    
    // Step 1: Check if RDB snapshot exists
    if (file_exists("dump.rdb")) {
        printf("Loading from snapshot...\n");
        db = db_load("dump.rdb");
    } else {
        db = hm_create(4096);
    }
    
    // Step 2: Replay AOF (if exists) to catch up
    aof_t* aof = aof_open("appendonly.aof", AOF_SYNC_EVERYSEC);
    if (aof) {
        printf("Replaying AOF...\n");
        aof_replay(aof, db);
    }
    
    // Step 3: Now database is fully recovered!
    printf("Database ready!\n");
    
    // Continue normal operation...
}