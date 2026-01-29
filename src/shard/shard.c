///@file:shard.c
#include <stdio.h>
#include <unistd.h>
#include "../hash/hash.h"
#include "../hashmap/hashmap.h"
#include "./ringbuffer/ringbuffer.h"
#include "../arena/arena.h"
#include <sched.h> // sched_yield()
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#define mod %
#define NPROC sysconf(_SC_NPROCESSORS_ONLN)
// take this number and half it
#define NSLAVE (NPROC/2)
#define NCOMMANDER 2 // why two coz i was thinking y not
// we take one write-commander and a read-comander

// for my system NSLAVE WILL Be 8 so 
// we have 8 hashmaps so it will ba cap of 8 * HM_CAP = 524288
// here instead of keeping this seperate lets call it
typedef struct{
    pthread_t thread;
    hashmap *map; // this will be the list of hashmap and also the sharded version
    // queue_t q; or rBuff;
    rBuff ring;
    arena* key_arena;
    arena* val_arena;
}shard_t;

shard_t* shards;

// void *shard_worker(void *arg){
//     shard_t *s = arg;

//     for(;;){
//         proc_t *p = rBuff_read(&s->ring);
//         if(!p){
//             sched_yield();
//             continue;
//         }
//         int rc = 0;
//         switch (p->op){
//         case OP_PUT:
//             rc = hm_insert(s->map, p->key, p->value, p->val_len);
//             break;
//         case OP_GET:
//             void *val = hm_get(s->map, p->key);
//             if(p->out_val){
//                 *p->out_val = val;
//             }
//             rc = (val != NULL);
//             break;
//         case OP_DEL:
//             rc = hm_delete(s->map, p->key, p->free_value);
//             break;
//         default:
//             rc = -1;
//             break;
//         }
//         if(p->status){
//             *p->status = rc;
//         }
//         atomic_store_explicit(&p->done, 1, memory_order_release);
//     }
// }

void* shard_worker(void* arg) {
    shard_t* s = arg;

    for (;;) {
        proc_t* p = rBuff_read(&s->ring);
        if (!p) continue;

        if (p->op == OP_PUT) {
            char* k = arena_alloc(s->key_arena, strlen(p->key) + 1);
            memcpy(k, p->key, strlen(p->key) + 1);

            void* v = arena_alloc(s->val_arena, p->val_len);
            memcpy(v, p->value, p->val_len);

            hm_insert(s->map, k, v, p->val_len);
        }
        else if (p->op == OP_GET) {
            *p->out_val = hm_get(s->map, p->key);
        }
        else if (p->op == OP_DEL) {
            hm_delete(s->map, p->key, p->free_value);
        }

        *p->status = 0;
        p->done = 1;
    }
}


void shard_init(){
    shards = malloc(NSLAVE * sizeof(shard_t));
    // threads = malloc(NSLAVE * sizeof(pthread_t));
    // maps = malloc(NSLAVE * sizeof(hashmap *));
    for(int i = 0; i < NSLAVE; i++){
        shard_t *s = &shards[i];
        s->map = hm_create(HM_CAP);
        s->key_arena = arena_create(8 * 1024 * 1024);  // 8MB
        s->val_arena = arena_create(32 * 1024 * 1024); // 32MB
        // s->ring = ring_init();
        rBuff_init(&s->ring, 1024);
        if(pthread_create(&s->thread, NULL, shard_worker, s) != 0) return;//abort
        
        // maps[i] = hm_create(HM_CAP);

    }
    // create the threads and
    // give it a hashmap
    // now we need a helper function that manages its
    // own hashmap
}
// void dispatch(const char* key){
//     uint64_t hash = FNV_1a(key, strlen(key));
//     int thread_id = hash % NSLAVE;

// }
void db_start(){
    printf("[Orca] DB started!!!");
    shard_init();
}

void db_insert(const char* key, void* value, size_t val_len){
    // take the key look for the correct thread
    // i am thinking to use a different hash function maybe
    // this could reduce collision
    uint64_t l0 = djb2(key, strlen(key));
    // for now we will let this be nslave but we have to optimize this too
    /// @todo optimize the below line
    int thread_id = l0 mod NSLAVE;
    // the below thread should take the key and insert into its hashmap;
    // threads[thread_id];
}