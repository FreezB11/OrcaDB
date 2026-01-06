///@file: server.c
#include "http/http.h"
#include "http/route.h"
#include <stdlib.h>
#include <stdio.h>

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
}

int main(){
    http* Orca = CreateServer();
    // localhost:8080/api/v1/PUT?key=<key>&value=<value>
    add_route("PUT","/api/v1/PUT", insert_handler);
}