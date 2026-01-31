#include "manager.h"

server_n servers[MAX_SERVER];
int server_count = 0;

void register_server(int fd, const char *buf, struct sockaddr_in cli) {
    if (server_count >= MAX_SERVER) {
        close(fd);
        return;
    }

    servers[server_count].fd = fd;
    strncpy(servers[server_count].id, buf + 7, 63); // after "SERVER "
    servers[server_count].addr = cli;

    printf("Registered server %s\n", servers[server_count].id);

    server_count++;
}

void handle_client(int fd, char *buf, size_t len) {
    if (server_count == 0) {
        const char *err = "No backend servers available\n";
        send(fd, err, strlen(err), 0);
        return;
    }

    int target = 0; // round-robin later
    send(servers[target].fd, buf, len, 0);
}

// void manager(){
//     int sock = socket(AF_INET, SOCK_STREAM, 0);

//     int opt = 1;
//     setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));


//     struct sockaddr_in addr;
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(8050);
//     addr.sin_addr.s_addr = INADDR_ANY;

//     if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0){
//         perror("bind");
//         return;
//     }
//     if(listen(sock, 10) < 0){
//         perror("listener");
//         return;
//     }
//     while(1){
//         struct sockaddr_in cli;
//         socklen_t len = sizeof(cli);
//         int fd = accept(sock, (struct sockaddr *)&cli, &len);

        

//         char buff[4096];
//         size_t n = recv(fd, buff, sizeof(buff), 0);

//         if (n > 0) {
//             write(STDOUT_FILENO, buff, n);
//         }

//         if (strncmp(buff, "SERVER", 6) == 0) {
//             register_server(fd, buff);
//         } else {
//             handle_client(fd, buff);
//         }

//         // for(int i = 0 ; i < 4096; i++){
//         //     printf("%c",buff[i]);
//         // }
//         int different_server = socket(AF_INET, SOCK_STREAM, 0);
//         printf("lets see if the different server socke tis opening\n");
//         struct sockaddr_in another = {0};
//         another.sin_family = AF_INET;
//         another.sin_port = htons(9001);

//         inet_pton(AF_INET, "127.0.0.1", &another.sin_addr);
//         connect(different_server, (struct sockaddr *)&another, sizeof(another));
//         size_t m = send(different_server, buff, n, 0);
//         printf("recieved = %ld, send = %ld", n, m);
//         printf("sending data to the other server\n");
    
//     close(fd);
//     close(sock);
//     close(different_server);
//     }
//     printf("\n");
//     return;
// }

void manager() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8050);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return;
    }

    if (listen(sock, 128) < 0) {
        perror("listen");
        return;
    }

    printf("Manager listening on 8050\n");

    while (1) {
        struct sockaddr_in cli = {0};
        socklen_t len = sizeof(cli);
        int fd = accept(sock, (struct sockaddr *)&cli, &len);

        if (fd < 0) continue;

        char buff[4096] = {0};
        ssize_t n = recv(fd, buff, sizeof(buff), 0);
        if (n <= 0) {
            close(fd);
            continue;
        }

        if (strncmp(buff, "SERVER", 6) == 0) {
            register_server(fd, buff, cli);
            send(fd, "OK\n", 3, 0);
            // DO NOT close fd
        } else {
            handle_client(fd, buff, n);
            close(fd); // clients are short-lived
        }
    }
}
