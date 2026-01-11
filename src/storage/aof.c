///@file: aof.c
#include "aof.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// here we will be implementing Append-on-File(AoF)
// the use is when we do a db_save we flush the AoF and then we 
// keep updating the new calls made by the client
// when crach occurs the db file is loaded and then the AoF 
// called to replay all the client calls, we have to keep 
// it thread safe coz we might have a race-condition

// we will save to .aof
// its a 2am code so i did not put the doxygen-com
// please bear with me

aof_t* aof_open(const char* filename, aof_sync_policy policy){
    if(!filename) return NULL;

    aof_t* aof = malloc(sizeof(aof_t));
    if(!aof) return NULL;

    aof->file = fopen(filename, "a+");
    if(!aof->file){
        perror("fopen");
        free(aof);
        return NULL;
    }
    aof->filename = strdup(filename);
    aof->sync_policy = policy;
    aof->last_sync = time(NULL);
    aof->bytes_written = 0;

    printf("[AOF] Opened %s (policy=%d)\n", filename, policy);
    return aof;
}

void aof_close(aof_t* aof){
    if(!aof) return ;
    if(aof->file){
        fflush(aof->file);
        fsync(fileno(aof->file));
        fclose(aof->file);
    }
    free(aof->filename);
    free(aof);
    printf("[AOF] closed\n");
}

// force sync
// static void aof_sync(aof_t* aof){
//     if(!aof || !!aof->file) return;
//     fflush(aof->file);
//     switch(aof->sync_policy){
//         case AOF_SYNC_ALWAYS:
//             fsync(fileno(aof->file));
//             break;
//         case AOF_SYNC_EVERYSEC:
//             time_t now = time(NULL);
//             if(now - aof->last_sync >= 1){
//                 fsync(fileno(aof->file));
//                 aof->last_sync = now;
//             }
//             break;
//         case AOF_SYNC_NO:
//             break;
//     }
// }
// patch v0.1.2
static void aof_sync(aof_t* aof){
    if (!aof || !aof->file) return;

    fflush(aof->file);

    switch (aof->sync_policy) {
        case AOF_SYNC_ALWAYS:
            fsync(fileno(aof->file));
            break;

        case AOF_SYNC_EVERYSEC: {
            time_t now = time(NULL);
            if (now - aof->last_sync >= 1) {
                fsync(fileno(aof->file));
                aof->last_sync = now;
            }
            break;
        }

        case AOF_SYNC_NO:
            break;
    }
}

/// @brief append the set command
int aof_append_set(aof_t* aof, const char* key, const void* value, size_t val_len){
    // printf("[LOG] we are in the aof_append_set <<-- this is a debug print\n");
    if (!aof || !aof->file || !key || !value) return -1;
    printf("[LOG] we are in the aof_append_set <<-- this is a debug print\n");
    
    size_t key_len = strlen(key);
    
    // Write command: SET key_len key val_len value\n
    int written = fprintf(aof->file, "SET %zu %s %zu ", key_len, key, val_len);
    printf("SET %zu %s %zu \n", key_len, key, val_len);
    if (written < 0) return -1;
    
    // Write value (may contain binary data or newlines)
    fwrite(value, 1, val_len, aof->file);
    fprintf(aof->file, "\n");
    
    aof->bytes_written += written + val_len + 1;
    
    aof_sync(aof);
    
    return 0;
}

/// @brief we write the del command to the aof file
/// @param aof 
/// @param key 
/// @return 
int aof_append_del(aof_t* aof, const char* key) {
    if (!aof || !aof->file || !key) return -1;
    
    size_t key_len = strlen(key);
    
    // Write command: DEL key_len key\n
    int written = fprintf(aof->file, "DEL %zu %s\n", key_len, key);
    if (written < 0) return -1;
    
    aof->bytes_written += written;
    
    aof_sync(aof);
    
    return 0;
}

/// @brief Replay AOF to rebuild database
/// @param aof AOF handle
/// @param hm Hashmap to populate
/// @return 0 on success, -1 on error
int aof_replay(aof_t* aof, hashmap* hm) {
    if (!aof || !aof->file || !hm) return -1;
    
    // Rewind to beginning
    rewind(aof->file);
    
    printf("[AOF] Replaying log...\n");
    time_t start = time(NULL);
    size_t commands = 0;
    
    char line[4096];
    while (fgets(line, sizeof(line), aof->file)) {
        char cmd[16];
        size_t key_len, val_len;
        
        // Parse command type
        if (sscanf(line, "%15s", cmd) != 1) {
            fprintf(stderr, "[AOF] Failed to parse command\n");
            continue;
        }
        
        if (strcmp(cmd, "SET") == 0) {
            // Parse: SET key_len key val_len value
            char* p = line + 4;  // Skip "SET "
            
            if (sscanf(p, "%zu", &key_len) != 1) {
                fprintf(stderr, "[AOF] Failed to parse key_len\n");
                continue;
            }
            
            // Find key
            p = strchr(p, ' ') + 1;
            char* key = malloc(key_len + 1);
            memcpy(key, p, key_len);
            key[key_len] = '\0';
            
            // Find value length
            p += key_len + 1;
            if (sscanf(p, "%zu", &val_len) != 1) {
                fprintf(stderr, "[AOF] Failed to parse val_len\n");
                free(key);
                continue;
            }
            
            // Find value
            p = strchr(p, ' ') + 1;
            void* value = malloc(val_len + 1);
            memcpy(value, p, val_len);
            ((char*)value)[val_len] = '\0';
            
            // Insert into hashmap
            hm_insert(hm, key, value, val_len);
            
            free(key);
            commands++;
        }
        else if (strcmp(cmd, "DEL") == 0) {
            // Parse: DEL key_len key
            char* p = line + 4;  // Skip "DEL "
            
            if (sscanf(p, "%zu", &key_len) != 1) {
                fprintf(stderr, "[AOF] Failed to parse key_len\n");
                continue;
            }
            
            p = strchr(p, ' ') + 1;
            char* key = malloc(key_len + 1);
            memcpy(key, p, key_len);
            key[key_len] = '\0';
            
            // Delete from hashmap
            void* old_val = hm_get(hm, key);
            if (old_val) {
                hm_delete(hm, key, 1);  // Free the value
            }
            
            free(key);
            commands++;
        }
    }
    
    time_t elapsed = time(NULL) - start;
    printf("[AOF] Replayed %zu commands in %ld seconds\n", commands, elapsed);
    
    return 0;
}

/// @brief Rewrite AOF (compaction)
/// @param aof AOF handle
/// @param hm Current hashmap state
/// @return 0 on success, -1 on error
int aof_rewrite(aof_t* aof, hashmap* hm) {
    if (!aof || !hm) return -1;
    
    printf("[AOF] Starting rewrite...\n");
    time_t start = time(NULL);
    
    // Create temp file
    char temp_file[256];
    snprintf(temp_file, sizeof(temp_file), "%s.tmp", aof->filename);
    
    FILE* temp_fp = fopen(temp_file, "w");
    if (!temp_fp) {
        perror("fopen");
        return -1;
    }
    
    // Write current state of entire hashmap
    size_t entries = 0;
    for (size_t i = 0; i < hm->capacity; i++) {
        hm_node* node = hm->buckets[i];
        
        while (node) {
            // Write SET command for each entry
            fprintf(temp_fp, "SET %zu %s %zu ", 
                    node->key_len, node->key, node->val_len);
            fwrite(node->value, 1, node->val_len, temp_fp);
            fprintf(temp_fp, "\n");
            
            entries++;
            node = node->next;
        }
    }
    
    fflush(temp_fp);
    fsync(fileno(temp_fp));
    fclose(temp_fp);
    
    // Close old AOF
    fclose(aof->file);
    
    // Atomic rename
    if (rename(temp_file, aof->filename) != 0) {
        perror("rename");
        unlink(temp_file);
        aof->file = fopen(aof->filename, "a+");  // Reopen old file
        return -1;
    }
    
    // Reopen new AOF
    aof->file = fopen(aof->filename, "a+");
    if (!aof->file) {
        perror("fopen");
        return -1;
    }
    
    aof->bytes_written = 0;
    
    time_t elapsed = time(NULL) - start;
    printf("[AOF] Rewrite complete: %zu entries, %ld seconds\n", 
           entries, elapsed);
    
    return 0;
}