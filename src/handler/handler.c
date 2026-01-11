///@file: handler.c
#include "handler.h"
#include <pthread.h>
#include "../http/http.h"
#include "../storage/aof.h"

extern pthread_mutex_t pool_mutex;
extern aof_t *global;
extern hashmap* db;

inline const char *
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

    // global = aof_open(AOF_FILE, AOF_SYNC_EVERYSEC);

    // Validate parameters
    if (!key || !value || key_len == 0) {
        res->status = 400;
        pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos,
                        "{ \"error\": \"missing key or value\" }");
        res->body_ptr = resp_body;
        res->body_len = pos;
        return;
    }
    /*
        insert to the database i.e. to the hashmap
        so for now if we reach till here lets say
        we were successfull
    */
    char *key_copy = malloc(key_len +1);
    memcpy(key_copy, key, key_len);
    key_copy[key_len]='\0';
    char *val_copy = malloc(val_len + 1);
    memcpy(val_copy, value, val_len);
    val_copy[val_len] = '\0';

    // uint64_t h = FNV_1a(key_copy, key_len);
    // pthread_mutex_t *lock = &stripes[h & (STRIPES - 1)];

    pthread_mutex_lock(&pool_mutex);
    hm_insert(db, key_copy, val_copy, val_len);
    printf("[LOG] we shall now set it in the appen_log file\n");
    int ret = aof_append_set(global, key_copy, (void *)val_copy, val_len);
    if(ret){
        printf("[LOG] we have a issue here coz we got a -1\n");
    }else{
        printf("[LOG] we didnt get a -1, but the issue lies on the function\n");
    }
    fflush(stdout);
    // aof_close(global);
    pthread_mutex_unlock(&pool_mutex);

    res->status = 200;
    pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos,
                            "{\"insert\": \"successful\"}");
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

    char key_copy[key_len+1];
    memcpy(key_copy, key, key_len);
    key_copy[key_len] = '\0';

    pthread_mutex_lock(&pool_mutex);
    char* value = (char*)hm_get(db, key_copy);
    pthread_mutex_unlock(&pool_mutex);

    if (!value) {
        res->status = 404;
        pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos,
                        "{\"error\":\"key not found\"}");
    } else {
        res->status = 200;
        pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos,
                        "{\"value\":\"%s\"}", value);
    }
    // res->status = 200;
    // pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos,
    //                         "{\"insert\": \"successful\"}");
    res->body_ptr = resp_body;
    res->body_len = pos;
    return;
}

void delete_handler(http_req_t *req, http_resp_t *res){
    static __thread char resp_body[RESP_BUFFER_SIZE];
    int pos = 0;
    int key_len = 0;
    const char* key = req_get_param(req, "key", &key_len);

    // global = aof_open(AOF_FILE, AOF_SYNC_EVERYSEC);

    if(!key || key_len == 0){
        res->status = 400;
        pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos,
                            "{\"error\":\"missing key\"}");
        res->body_ptr = resp_body;
        res->body_len = pos;
        return;
    }

    char key_copy[key_len+1];
    memcpy(key_copy, key, key_len);
    key_copy[key_len] = '\0';

    pthread_mutex_lock(&pool_mutex);
    // char* value = (char*)hm_get(db, key_copy);
    //delete operation
    int value  = hm_delete(db, key_copy, 1);
    // printf("[debug-log] umm this is not working %s\n", key_copy);
    printf("[LOG] we shall now set it in the appen_log file\n");
    aof_append_del(global, key_copy);
    // aof_close(global);
    pthread_mutex_unlock(&pool_mutex);

    if (value) {
        res->status = 404;
        pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos,
                        "{\"error\":\"key not found\"}");
    } else {
        res->status = 200;
        pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos,
                        "{key deleted}", value);
    }
    // res->status = 200;
    // pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos,
    //                         "{\"insert\": \"successful\"}");
    res->body_ptr = resp_body;
    res->body_len = pos;
    return;
}