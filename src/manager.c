#include "manager.h"
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct servers{
    char* ip;
    char* port;
}n_server;

n_server one = {.ip = "0.0.0.0", .port="9001"};


void manager(){
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));


    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8050);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("bind");
        return;
    }
    if(listen(sock, 10) < 0){
        perror("listener");
        return;
    }
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);
    int cli_fd = accept(sock, (struct sockaddr *)&cli, &len);
    char buff[4096];
    size_t n = recv(cli_fd, buff, sizeof(buff), 0);

    if (n > 0) {
        write(STDOUT_FILENO, buff, n);
    }
    // for(int i = 0 ; i < 4096; i++){
    //     printf("%c",buff[i]);
    // }
    int different_server = socket(AF_INET, SOCK_STREAM, 0);
    printf("lets see if the different server socke tis opening\n");
    struct sockaddr_in another = {0};
    another.sin_family = AF_INET;
    another.sin_port = htons(9001);
    
    inet_pton(AF_INET, "127.0.0.1", &another.sin_addr);
    connect(different_server, (struct sockaddr *)&another, sizeof(another));
    size_t m = send(different_server, buff, n, 0);
    printf("recieved = %ld, send = %ld", n, m);
    printf("sending data to the other server\n");

    close(cli_fd);
    close(sock);
    close(different_server);

    printf("\n");
    return;
}