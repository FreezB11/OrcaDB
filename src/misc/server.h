///@file: server.h
#pragma once
#include <stdint.h>

void handshake(const char* ip, uint16_t port, const char *server_id);
void server();