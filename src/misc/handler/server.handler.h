///@file: handler.h
#pragma once
#include "../http/http.h"

/*
    this is not a major thing just wanted all the handlers to be here 
    nothing more, just to reduce no of lines in server.c file
*/

void insert_handler(http_req_t *req, http_resp_t *res);
void get_handler(http_req_t *req, http_resp_t *res);
void delete_handler(http_req_t *req, http_resp_t *res);