///@file: www.c
#include "../server.h"
#include "../manager.h"
// we will write he code for manager and 
// server side
// i will build this block by block and on my own 
// the goal is to make it exist then move ahead
#ifdef SERVER
    int main(int argc, char* argv[]){
        // dummy command
        // ./orca-server -p=8080 -ip=[manager-ip]
        // when the server is up or when we run the above command 
        // it must connect to the manager
        server();
    }
#endif

#ifdef MANAGER
    int main(int agrc, char* argv[]){
        // we will call
        // all the manager function that we need
        // we will start the manager here
        // ./orca-manager -p 9000
        manager();
        // for(;;) sleep(1);
    }
#endif