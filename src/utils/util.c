///@file: util.c
#include "util.h"
#include <sys/stat.h>
#include <stdbool.h>

bool file_exists(const char* filename){
    struct stat buffer;
    return stat(filename, &buffer) == 0 ? true : false;
}