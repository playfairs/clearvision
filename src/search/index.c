#include "index.h"
#include "../fs/fs.h"
#include "../util/memory.h"
#include "../util/string.h"
#include "../util/util.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct cv_index {
    char* path;
    char* db_path;
    sqlite3* db;
};

typedef struct {
    cv_index_t* index;
    size_t file_count;
    size_t total_size;
} index_context_t;

static int index_callback(void* user_data, int argc, char** argv, char** col_names) {
    (void)argc;
    (void)col_names;
    index_context_t* ctx = (index_context_t*)user_data;
    ctx->file_count++;
    if (argv[0]) {
        ctx->total_size += (size_t)atoll(argv[0]);
    }
    return 0;
}

static void file_index_callback(cv_file_info_t* file, void* user_data) {
    index_context_t* ctx = (index_context_t*)user_data;
    
    if (file->is_dir || file->is_binary) {
        return;
    }

    const char* sql = "INSERT OR REPLACE INTO files (path, name, size, modified) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(ctx->index->db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return;
    }

    sqlite3_bind_text(stmt, 1, file->path, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, file->name, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, (sqlite3_int64)file->size);
    sqlite3_bind_int64(stmt, 4, (sqlite3_int64)file->modified_time);
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    ctx->file_count++;
    ctx->total_size += file->size;
}

cv_index_t* cv_index_create(const char* path) {
    if (!path) {
        return NULL;
    }

    cv_index_t* index = cv_calloc(1, sizeof(cv_index_t));
    index->path = cv_strdup(path);

    char* cache_dir = cv_get_cache_dir();
    index->db_path = cv_string_append(cache_dir, "/clearvision.db");
    cv_free(cache_dir);

    if (sqlite3_open(index->db_path, &index->db) != SQLITE_OK) {
        cv_index_destroy(index);
        return NULL;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS files ("
                      "path TEXT PRIMARY KEY, "
                      "name TEXT, "
                      "size INTEGER, "
                      "modified INTEGER, "
                      "indexed INTEGER DEFAULT 1)";

    char* err_msg = NULL;
    if (sqlite3_exec(index->db, sql, NULL, NULL, &err_msg) != SQLITE_OK) {
        sqlite3_free(err_msg);
        cv_index_destroy(index);
        return NULL;
    }

    return index;
}

void cv_index_destroy(cv_index_t* index) {
    if (!index) {
        return;
    }
    if (index->db) {
        sqlite3_close(index->db);
    }
    cv_free(index->path);
    cv_free(index->db_path);
    cv_free(index);
}

int cv_index_build(cv_index_t* index) {
    if (!index || !index->db) {
        return -1;
    }

    const char* sql = "DELETE FROM files";
    sqlite3_exec(index->db, sql, NULL, NULL, NULL);

    cv_fs_options_t* options = cv_fs_options_create();
    index_context_t ctx = {index, 0, 0};
    
    cv_fs_traverse(index->path, options, file_index_callback, &ctx);
    
    cv_fs_options_destroy(options);

    return 0;
}

int cv_index_rebuild(cv_index_t* index) {
    return cv_index_build(index);
}

int cv_index_remove(cv_index_t* index) {
    if (!index || !index->db) {
        return -1;
    }

    const char* sql = "DELETE FROM files";
    char* err_msg = NULL;
    
    if (sqlite3_exec(index->db, sql, NULL, NULL, &err_msg) != SQLITE_OK) {
        sqlite3_free(err_msg);
        return -1;
    }

    return 0;
}

int cv_index_search(cv_index_t* index, const char* pattern, char*** results, size_t* count) {
    if (!index || !index->db || !pattern || !results || !count) {
        return -1;
    }

    const char* sql = "SELECT path FROM files WHERE path LIKE ? OR name LIKE ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(index->db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }

    char* pattern_with_wildcards = cv_string_append(cv_string_append(cv_strdup("%"), pattern), "%");
    sqlite3_bind_text(stmt, 1, pattern_with_wildcards, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, pattern_with_wildcards, -1, SQLITE_STATIC);

    size_t capacity = 256;
    *results = cv_malloc(capacity * sizeof(char*));
    *count = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* path = (const char*)sqlite3_column_text(stmt, 0);
        
        if (*count >= capacity) {
            capacity *= 2;
            *results = cv_realloc(*results, capacity * sizeof(char*));
        }
        
        (*results)[*count] = cv_strdup(path);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    cv_free(pattern_with_wildcards);

    return 0;
}

int cv_index_get_stats(cv_index_t* index, char* stats, size_t size) {
    if (!index || !index->db || !stats) {
        return -1;
    }

    index_context_t ctx = {index, 0, 0};
    const char* sql = "SELECT COUNT(*), SUM(size) FROM files";
    char* err_msg = NULL;
    
    if (sqlite3_exec(index->db, sql, index_callback, &ctx, &err_msg) != SQLITE_OK) {
        sqlite3_free(err_msg);
        return -1;
    }

    snprintf(stats, size, "Files: %zu\nTotal size: %zu bytes", ctx.file_count, ctx.total_size);
    return 0;
}
