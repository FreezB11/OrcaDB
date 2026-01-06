#include "./lru_cache/lru_cache.h"
#include <stdio.h>

int main() {
    lru_cache* c = lru_create(2);

    lru_put(c, "a", "1");
    lru_put(c, "b", "2");

    printf("a = %s\n", lru_get(c, "a")); // 1

    lru_put(c, "c", "3"); // evicts b

    printf("b = %s\n", lru_get(c, "b")); // NULL
    printf("c = %s\n", lru_get(c, "c")); // 3
    printf("a = %s\n", lru_get(c, "a")); // 1

    lru_free(c);
    return 0;
}
