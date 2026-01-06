#include <http/http.h>
#include <http/route.h>
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

int main(){
    http *App = CreateServer();
    add_route("GET","/api/v1/GET/:key", handle_echo);
    add_route("GET","/api/v1/PUT/key&value", handle_ping);
    if (!App) {
        fprintf(stderr, "Failed to create server\n");
        return 1;
    }
    
    printf("✅ Server running! Press Ctrl+C to stop.\n\n");
    fflush(stdout);
    
    for(;;) sleep(1);

    return HTTP_STATS;
}

/*
: 1765966243:0;gcc -O3 -I include -march=native -flto -pthread -D_GNU_SOURCE main.c ./src/*.c -o http
: 1765966245:0;./http
*/