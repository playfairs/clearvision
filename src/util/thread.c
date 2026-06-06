#include "thread.h"
#include "memory.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>

typedef struct cv_task {
    void (*func)(void*);
    void* arg;
    struct cv_task* next;
} cv_task_t;

typedef struct cv_thread_pool {
    pthread_t* threads;
    size_t thread_count;
    atomic_int shutdown;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    cv_task_t* task_queue;
    atomic_int pending_tasks;
    pthread_cond_t done;
} cv_thread_pool_t;

static void* worker_thread(void* arg) {
    cv_thread_pool_t* pool = (cv_thread_pool_t*)arg;
    
    while (1) {
        pthread_mutex_lock(&pool->mutex);
        
        while (!pool->task_queue && !atomic_load(&pool->shutdown)) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }
        
        if (atomic_load(&pool->shutdown)) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        
        cv_task_t* task = pool->task_queue;
        pool->task_queue = task->next;
        pthread_mutex_unlock(&pool->mutex);
        
        task->func(task->arg);
        cv_free(task);
        
        if (atomic_fetch_sub(&pool->pending_tasks, 1) == 1) {
            pthread_cond_signal(&pool->done);
        }
    }
    
    return NULL;
}

cv_thread_pool_t* cv_thread_pool_create(size_t thread_count) {
    if (thread_count == 0) {
        thread_count = 1;
    }
    
    cv_thread_pool_t* pool = cv_calloc(1, sizeof(cv_thread_pool_t));
    pool->thread_count = thread_count;
    pool->threads = cv_malloc(thread_count * sizeof(pthread_t));
    
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond, NULL);
    pthread_cond_init(&pool->done, NULL);
    atomic_init(&pool->shutdown, 0);
    atomic_init(&pool->pending_tasks, 0);
    
    for (size_t i = 0; i < thread_count; i++) {
        pthread_create(&pool->threads[i], NULL, worker_thread, pool);
    }
    
    return pool;
}

void cv_thread_pool_destroy(cv_thread_pool_t* pool) {
    if (!pool) {
        return;
    }
    
    atomic_store(&pool->shutdown, 1);
    pthread_cond_broadcast(&pool->cond);
    
    for (size_t i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);
    pthread_cond_destroy(&pool->done);
    
    cv_task_t* task = pool->task_queue;
    while (task) {
        cv_task_t* next = task->next;
        cv_free(task);
        task = next;
    }
    
    cv_free(pool->threads);
    cv_free(pool);
}

void cv_thread_pool_submit(cv_thread_pool_t* pool, void (*func)(void*), void* arg) {
    if (!pool || !func) {
        return;
    }
    
    cv_task_t* task = cv_malloc(sizeof(cv_task_t));
    task->func = func;
    task->arg = arg;
    task->next = NULL;
    
    pthread_mutex_lock(&pool->mutex);
    
    if (pool->task_queue) {
        cv_task_t* tail = pool->task_queue;
        while (tail->next) {
            tail = tail->next;
        }
        tail->next = task;
    } else {
        pool->task_queue = task;
    }
    
    atomic_fetch_add(&pool->pending_tasks, 1);
    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);
}

void cv_thread_pool_wait(cv_thread_pool_t* pool) {
    if (!pool) {
        return;
    }
    
    pthread_mutex_lock(&pool->mutex);
    while (atomic_load(&pool->pending_tasks) > 0) {
        pthread_cond_wait(&pool->done, &pool->mutex);
    }
    pthread_mutex_unlock(&pool->mutex);
}
