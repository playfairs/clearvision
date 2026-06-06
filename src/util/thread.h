#ifndef CLEARVISION_UTIL_THREAD_H
#define CLEARVISION_UTIL_THREAD_H

#include <stddef.h>

typedef struct cv_thread_pool cv_thread_pool_t;

cv_thread_pool_t* cv_thread_pool_create(size_t thread_count);
void cv_thread_pool_destroy(cv_thread_pool_t* pool);
void cv_thread_pool_submit(cv_thread_pool_t* pool, void (*func)(void*), void* arg);
void cv_thread_pool_wait(cv_thread_pool_t* pool);

#endif
