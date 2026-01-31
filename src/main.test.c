///@file:main.test.c
#include "orca/orca.h"

///@todo this is the goal we must achieve
int main(){
    Orca* db = orca_init();
    char* key = "";
    char* val = "";
    db.insert(key, val);
    char* val_test;
    val_test = db.get(key);
}