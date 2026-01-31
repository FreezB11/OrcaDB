#include "http.h"
#include "route.h"
#include <stdio.h>
#include <string.h>

route_t routes[128];
int route_count = 0;

void handle_ping(http_req_t *req, http_resp_t *res) {
    (void)req;
    static __thread char json[] = "{\"status\":\"ok\"}";
    res->is_static = 1; 
    res->status = 200;
    res->body_ptr = json;
    res->body_len = strlen(json);
}


void handle_echo(http_req_t *req, http_resp_t *res) {
    // printf("[LOG]: the user hit the /echo route\n");
    static __thread char resp_body[RESP_BUFFER_SIZE];
    int pos = 0;
    pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos, "{");
    for (int i = 0; i < req->param_count; i++) {
        if (i > 0) pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos, ",");
        pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos,
                        "\"%.*s\":\"%.*s\"",
                        req->params[i].key_len, req->params[i].key,
                        req->params[i].val_len, (char *)req->params[i].val);
    }
    pos += snprintf(resp_body + pos, RESP_BUFFER_SIZE - pos, "}");
    res->status = 200;
    res->body_ptr = resp_body;
    res->body_len = pos;
}

void handle_404(http_req_t *req, http_resp_t *res) {
    (void)req;
    // printf("[LOG]: this is a site not found error code = 404\n");
    static const char json[] = "{\"error\":\"Not Found\"}";
    res->status = 404;
    res->body_ptr = (char *)json;
    res->body_len = sizeof(json) - 1;
}

void add_route(const char *method, const char *path, route_handler_t handler){
    if(route_count >= 128) return;
    routes[route_count++] = (route_t){method, path, handler};
}

void route_req(http_req_t *req, http_resp_t *res) {
    for (int i = 0; i < route_count; i++) {
        if (req->method_len == (int)strlen(routes[i].method) &&
            strncmp(req->method, routes[i].method, req->method_len) == 0 &&
            req->path_len == (int)strlen(routes[i].path) &&
            strncmp(req->path, routes[i].path, req->path_len) == 0) {
            routes[i].handler(req, res);
            return;
        }
    }
    // printf("this is is a 404 error\n");
    handle_404(req, res);
}