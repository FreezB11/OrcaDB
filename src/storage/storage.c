///@file: storage.c
// the goal is simple take the hashmap and 
// store it to the binary format
// lets the file be called .orca
#include "../hashmap/hashmap.h"
#include "storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#define DB_VERSION 1
#define DB_MAGIC "OrcaDB"

/// @brief a crc32 checksum to assure the data intactness 
static uint32_t checksum(const char* data, size_t len){
    uint32_t sum = 0;
    for(size_t i = 0; i < len; i++){
        sum = sum * 31 + (unsigned char)data[i];
    }
    return sum;
}

/*
    doc/manual
    size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
    ptr — where the data lives
    size — size of ONE element (in bytes)
    nmemb — number of elements
    stream — where the data goes
*/

/// @brief function to snapshot like take picture of current state of our hashmap to disk
/// @param hm - hashmap
/// @param filename - filename where it will stored
/// @return -1 on failure, 0 on success
int db_save(hashmap* hm, const char* filename){
    /*
        file format
        ===============================
        [header]
        -magic (7bytes)
        -version (4bytes)
        -timestap: when it was saved(8bytes)
        -count: no of entries

        [Entries]
        -key length(8bytes)
        -key data(key_len bytes)
        -value length(8bytes)
        -value data(val_len bytes)

        [footer]
        -checksum:crc32(4bytes)
    */
    if(!hm || !filename) return -1;
    size_t CAP = hm->capacity;
    size_t count = hm->count;

    //we gota keep it smart and avoid
    // partial snapshot that is what if it crashed 
    // during write then we have corrupted data so 
    // we write to temp and then rename and save
    char temp_file[1024];
    snprintf(temp_file, sizeof(temp_file), "%s.tmp", filename);

    FILE *file = fopen(temp_file, "wb");
    if(!file){
        perror("fopen");
        return -1;
    }
    printf("[Orca] Starting snapshot to %s...\n", filename);
    time_t start = time(NULL);

    // we will write the header that is magic version timestamps and count
    fwrite(DB_MAGIC, 1, 7, file);
    uint32_t version = DB_VERSION;
    fwrite(&version, sizeof(version), 1, file);
    uint64_t timestamp = (uint64_t)time(NULL);
    fwrite(&timestamp, sizeof(timestamp), 1, file);
    fwrite(&count, sizeof(count), 1, file);

    uint32_t _checksum = 0;
    size_t entries_written = 0;

    for(size_t i = 0; i < CAP; i++){
        hm_node* node = hm->buckets[i];
        while(node){
            // we write the key_len
            uint32_t key_len = (uint32_t)node->key_len;
            fwrite(&key_len, sizeof(key_len), 1, file);
            fwrite(node->key, 1, key_len, file);
            _checksum += checksum(node->key, key_len);

            //value time
            uint32_t val_len = (uint32_t)node->val_len;
            fwrite(&val_len, sizeof(val_len), 1, file);
            if(node->value && val_len > 0){
                fwrite(node->value, 1, val_len, file);
                _checksum += checksum(node->value, val_len);                
            }
            entries_written++;
            node = node->next;
        }
    }

    // write the checksum
    fwrite(&_checksum, sizeof(_checksum), 1, file);
    fclose(file);
    // now we will rename
    // which will ensure consistency
    if(rename(temp_file, filename) != 0){
        perror("rename");
        unlink(temp_file);
        return -1;
    }
    time_t elapsed = time(NULL) - start;
    printf("[Orca] Snapshot complete: %zu entries, %ld seconds\n", entries_written, elapsed);

    return 0;
}

/// @brief load from the disk
/// @param filename the db file
/// @return loaded hashmap, or null on error
hashmap* db_load(const char* filename){
    if(!filename) return NULL;

    FILE* file = fopen(filename, "rb");
    if(!file){
        perror("fopen\n");
        return NULL;
    }
    printf("[Orca] Loading snapshot from %s....\n", filename);
    time_t start = time(NULL);


    char magic[6];
    fread(&magic,1, 6, file);
    if(memcmp(DB_MAGIC, magic, 6) != 0){
        fprintf(stderr, "[Orca] Invalid magic number\n");
        // perror("magic not same\n");
        fclose(file);
        return NULL;
    }

    uint32_t version;
    fread(&version, sizeof(version), 1, file);
    if (version != DB_VERSION) {
        fprintf(stderr, "[RDB] Unsupported version: %u\n", version);
        fclose(file);
        return NULL;
    }

    uint64_t timestamp;
    fread(&timestamp, sizeof(timestamp), 1, file);
    uint64_t count;
    fread(&count, sizeof(count), 1, file); 

    size_t capacity = 2*count; // 2x for lower load factor
    if(capacity < 1024) capacity = 1024;

    hashmap* hm = hm_create(capacity);
    if(!hm){
        fclose(file);
        return NULL;
    }
    uint32_t _checksum = 0;
    for(size_t i = 0; i < count; i++){
        uint32_t key_len;
        if(fread(&key_len, sizeof(key_len), 1, file) != 1){
            fprintf(stderr, "[Orca] Failed to read key length\n");
            hm_free(hm, 1);
            fclose(file);
            return NULL;
        }
        char* key = malloc(key_len + 1);
        if(!key || fread(key, 1, key_len, file) != key_len){
            fprintf(stderr, "[Orca] Failed to read key data\n");
            free(key);
            hm_free(hm, 1);
            fclose(file);
            return NULL;
        }
        key[key_len] = '\0';
        _checksum += checksum(key, key_len);

        // read val len
        uint32_t val_len;
        if(fread(&val_len, sizeof(val_len), 1, file) != 1){
            fprintf(stderr, "[Orca] Failed to read value length\n");
            free(key);
            hm_free(hm, 1);
            fclose(file);
            return NULL;
        }
        // read value
        void* value = NULL;
        if(val_len > 0){
            value = malloc(val_len + 1);
            if(!value || fread(value, 1, val_len, file) != val_len){
                fprintf(stderr, "[Orca] Failed to read value\n");
                free(key);
                free(value);
                hm_free(hm, 1);
                fclose(file);
                return NULL;
            }
            ((char*)value)[val_len] = '\0';
            _checksum += checksum(value,val_len);
        }
        hm_insert(hm, key, value, val_len);
        free(key);
    }
    uint32_t stored_checksum;
    fread(&stored_checksum, sizeof(stored_checksum), 1, file);
    if(_checksum != stored_checksum){
        fprintf(stderr, "[Orca] Checksum mismatch! File may be corrupted.\n");
        fprintf(stderr, "[Orca] Calculated: %u, Stored: %u\n", 
                _checksum, stored_checksum);
    }
    fclose(file);
    time_t elapsed = time(NULL) - start;
    printf("[Orca] Loaded %lu entries in %ld seconds\n", count, elapsed);

    return hm;
}

int db_save_proc(hashmap* hm, const char* filename){
    #ifdef _WIN32
    fprintf(stderr, "[Orca] background save not supported on windows\n");
    return db_save(hm, filename);
    #else
    pid_t pid = fork();
    if(pid == -1){
        perror("fork");
        return -1;
    }
    if(pid == 0){
        int ret = db_save(hm, filename);
        exit(ret == 0 ? 0:1);
    }
    printf("[Orca] background save started (pid=%d)\n", pid);
    return 0;
    #endif
}