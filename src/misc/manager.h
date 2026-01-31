///@file: manager.h
#pragma once
#include "./http/http.h"
#include "./http/parse.h"
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_SERVER 20

typedef struct server{
    int fd;
    char id[64];
    struct sockaddr_in addr;
}server_n;

extern server_n servers[MAX_SERVER];
extern int server_count;

/*
=== dummy message == 
TYPE=SERVER
ID=server-1
CAPS=storage,compute
*/

// void *manager_thread(void *arg);
// http* CreateManager();
void handle_fd();
void manager();