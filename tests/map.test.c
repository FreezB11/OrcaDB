#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/hashmap/hashmap.h"

int main() {
    hashmap* hm = hm_create(1024);
    
    // Test insert with string values
    // Since value is void*, we pass the string pointer directly
    char* alice = strdup("Alice");
    char* bob = strdup("Bob");
    
    hm_insert(hm, "user:123", alice, strlen(alice));
    hm_insert(hm, "user:456", bob, strlen(bob));
    
    // Test get
    char* val = (char*)hm_get(hm, "user:123");
    printf("user:123 = %s\n", val ? val : "NULL");
    
    // Test update
    char* alice_updated = strdup("Alice Updated");
    // Note: We need to free the old value before updating
    char* old_val = (char*)hm_get(hm, "user:123");
    hm_insert(hm, "user:123", alice_updated, strlen(alice_updated));
    free(old_val);  // Free the old "Alice" string
    
    val = (char*)hm_get(hm, "user:123");
    printf("user:123 = %s\n", val ? val : "NULL");
    
    // Test delete with free_value flag
    hm_delete(hm, "user:456", 1);  // 1 = free the value pointer
    val = (char*)hm_get(hm, "user:456");
    printf("user:456 = %s (should be NULL)\n", val ? val : "NULL");
    
    // Clean up - free_values=1 will free remaining value pointers
    hm_free(hm, 1);
    return 0;
}