#ifndef CLEARVISION_SEARCH_INDEX_H
#define CLEARVISION_SEARCH_INDEX_H

#include <stddef.h>

typedef struct cv_index cv_index_t;

cv_index_t* cv_index_create(const char* path);
void cv_index_destroy(cv_index_t* index);
int cv_index_build(cv_index_t* index);
int cv_index_rebuild(cv_index_t* index);
int cv_index_remove(cv_index_t* index);
int cv_index_search(cv_index_t* index, const char* pattern, char*** results, size_t* count);
int cv_index_get_stats(cv_index_t* index, char* stats, size_t size);

#endif
