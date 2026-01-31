///@file: server.c
#include "./http/http.h"
#include "./http/route.h"
#include "./hashmap/hashmap.h"
#include "./hash/hash.h"
#include "./storage/aof.h"
#include "./storage/storage.h"
#include "./storage/aof.h"
#include "./handler/server.handler.h"
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
// #include <sys/stat.h>
// #include <stdbool.h>
#include "./utils/util.h"

#define STRIPES 64
#define AOF_FILE "append_log.aof"
#define DB_FILE "data.orca"
#define SNAPSHOT_INTERVAL 30 // for testing let it be 30 seconds for production it will be bigger maybe i have to test it

aof_t *global = NULL;
hashmap* db = NULL;
conn_ctx_t *conn_pool = NULL;
int conn_pool_next = 0;
pthread_mutex_t pool_mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t stripes[STRIPES];

/*
    what we need now is we have to append and then 
    flush the append_log.aof when we do a db_save and then on start we load that then do the 
    replay for append log which will keep the system up to date    

    but we must do it periodically
*/

void *snapshot_worker(void *arg) {
    (void)arg;

    for (;;) {
        sleep(SNAPSHOT_INTERVAL);

        printf("[Orca] Snapshotting database...\n");

        pthread_mutex_lock(&pool_mutex);

        // Save DB snapshot
        if (db_save(db, DB_FILE) != 0) {
            fprintf(stderr, "[Orca] db_save failed\n");
        } else {
            printf("[Orca] Snapshot saved\n");
        }
        aof_rewrite(global, db);

        pthread_mutex_unlock(&pool_mutex);
    }

    return NULL;
}

// void handshake(const char* ip, uint64_t port){
//     printf("we will proceed with handshake\n");
//     int temp_sock = socket(AF_INET, SOCK_STREAM, 0);
//     // assigning the manager
//     struct sockaddr_in manager = {0};
//     manager.sin_family = AF_INET;
//     manager.sin_port = htons(port);
//     inet_pton(AF_INET, ip, &manager.sin_addr);

//     //now we connect to the manager telling them we exist
//     connect(temp_sock, (struct sockaddr *)&manager ,sizeof(manager));

//     char buff[4096] = {0};
//     char *src = " this is a secret message\n";
//     memcpy(&buff, src, strlen(src));
//     send(temp_sock, buff, strlen(src), 0);
// }  

void handshake(const char* ip, uint16_t port, const char *server_id) {
    int temp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (temp_sock < 0) {
        perror("socket");
        return;
    }

    struct sockaddr_in manager = {0};
    manager.sin_family = AF_INET;
    manager.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &manager.sin_addr) != 1) {
        perror("inet_pton");
        close(temp_sock);
        return;
    }

    if (connect(temp_sock, (struct sockaddr *)&manager, sizeof(manager)) < 0) {
        perror("connect");
        close(temp_sock);
        return;
    }

    char buf[256];
    snprintf(buf, sizeof(buf), "SERVER %s\n", server_id);

    send(temp_sock, buf, strlen(buf), 0);

    /* optional: wait for ACK */
    recv(temp_sock, buf, sizeof(buf), 0);

    /* DO NOT close temp_sock */
    /* keep it open — this is your control channel */
}


void server(){
    // for (int i = 0; i < STRIPES; i++)
        // pthread_mutex_init(&stripes[i], NULL);

    // hashmap* db = NULL;
    http* Orca = CreateServer();
    global = aof_open(AOF_FILE, AOF_SYNC_EVERYSEC);

    if(file_exists(DB_FILE)){
        printf("[Orca] loading from snapshots...\n");
        db = db_load(DB_FILE);
    }else{
        db = hm_create(4096 * 16 * 16);
    }

    // aof_t* aof = aof_open(AOF_FILE, AOF_SYNC_EVERYSEC);
    if(global){
        printf("[Orca] Replaying AOF...\n");
        aof_replay(global, db);
    }

    // pthread_t snapshot_thread;
    // pthread_create(&snapshot_thread, NULL, snapshot_worker, NULL);
    // pthread_detach(snapshot_thread);

    printf("[Orca] Database ready!!!!\n");
    add_route("PUT","/api/v1/PUT", insert_handler);
    add_route("GET","/api/v1/GET", get_handler);
    add_route("DELETE","/api/v1/DELETE", delete_handler);

    printf("Server running! Press Ctrl+C to stop.\n\n");
    fflush(stdout);
    
    for(;;) sleep(1);
}

// int main(){
//     server();
// }