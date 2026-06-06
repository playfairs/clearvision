#include "memory.h"
#include <stdlib.h>
#include <string.h>

void* cv_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr && size > 0) {
        abort();
    }
    return ptr;
}

void* cv_calloc(size_t num, size_t size) {
    void* ptr = calloc(num, size);
    if (!ptr && num > 0 && size > 0) {
        abort();
    }
    return ptr;
}

void* cv_realloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (!new_ptr && size > 0) {
        abort();
    }
    return new_ptr;
}

char* cv_strdup(const char* s) {
    if (!s) {
        return NULL;
    }
    size_t len = strlen(s) + 1;
    char* copy = cv_malloc(len);
    memcpy(copy, s, len);
    return copy;
}

void cv_free(void* ptr) {
    free(ptr);
}
