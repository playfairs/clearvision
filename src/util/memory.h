#ifndef CLEARVISION_UTIL_MEMORY_H
#define CLEARVISION_UTIL_MEMORY_H

#include <stddef.h>

void* cv_malloc(size_t size);
void* cv_calloc(size_t num, size_t size);
void* cv_realloc(void* ptr, size_t size);
char* cv_strdup(const char* s);
void cv_free(void* ptr);

#endif
