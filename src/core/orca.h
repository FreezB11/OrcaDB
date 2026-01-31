///@file:orca.h

typedef struct{
    void (*insert)(const char* key, void *val);
    void* (*get)(const char* key);
}Orca;

Orca* orca_init();