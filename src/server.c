///@file: server.c
#include "./http/http.h"
#include "./http/route.h"
#include "./hashmap/hashmap.h"
#include "./storage/aof.h"
#include "./storage/storage.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdbool.h>

bool file_exists(const char *filename) {
    struct stat buffer;
    return stat(filename, &buffer) == 0 ? true : false;
}   
conn_ctx_t *conn_pool = NULL;
int conn_pool_next = 0;
pthread_mutex_t pool_mutex = PTHREAD_MUTEX_INITIALIZER;

static inline const char *
req_get_param(http_req_t *req, const char *key, int *val_len) {
    int klen = strlen(key);

    for (int i = 0; i < req->param_count; i++) {
        if (req->params[i].key_len == klen &&
            memcmp(req->params[i].key, key, klen) == 0) {
            if (val_len) *val_len = req->params[i].val_len;
            return req->params[i].val;
        }
    }
    return NULL;
}

void insert_handler(http_req_t *req, http_resp_t *res){
    static __thread char resp_body[RESP_BUFFER_SIZE];
    int pos = 0;
    int key_len = 0, val_len = 0;
    const char* key = req_get_param(req, "key", &key_len);
    const char* value = req_get_param(req, "value", &val_len);

    // Validate parameters
    if (!key || !value || key_len == 0) {
        res->status = 400;
        pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos,
                        "{ \"error\": \"missing key or value\" }");
        res->body_ptr = resp_body;
        res->body_len = pos;
        return;
    }
    printf("handler works here\n"); // <= test line will be removed soon
    /*
        insert to the database i.e. to the hashmap
        so for now if we reach till here lets say
        we were successfull
    */
    res->status = 200;
    pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos,
                            "{\"insert\": \"successful\"}\0");
    res->body_ptr = resp_body;
    res->body_len = pos;
    return;
}

void get_handler(http_req_t *req, http_resp_t *res){
    static __thread char resp_body[RESP_BUFFER_SIZE];
    int pos = 0;
    int key_len = 0;
    const char* key = req_get_param(req, "key", &key_len);

    if(!key || key_len == 0){
        res->status = 400;
        pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos,
                            "{\"error\":\"missing key\"}");
        res->body_ptr = resp_body;
        res->body_len = pos;
        return;
    }
}

int main(){
    hashmap* db = NULL;
    http* Orca = CreateServer();

    if(file_exists("data.orca")){
        printf("[Orca] loading from snapshots...\n");
        db = db_load("data.orca");
    }else{
        db = hm_create(4096);
    }

    aof_t* aof = aof_open("append_log.aof", AOF_SYNC_EVERYSEC);
    if(aof){
        printf("[Orca] Replaying AOF...\n");
        aof_replay(aof, db);
    }
    printf("[Orca] Database ready!!!!\n");
    add_route("PUT","/api/v1/PUT", insert_handler);
    add_route("GET","/api/v1/GET", get_handler);

    printf("✅ Server running! Press Ctrl+C to stop.\n\n");
    fflush(stdout);
    
    for(;;) sleep(1);
}