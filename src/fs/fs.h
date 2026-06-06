#ifndef CLEARVISION_FS_FS_H
#define CLEARVISION_FS_FS_H

#include <stddef.h>
#include <time.h>

typedef struct cv_file_info {
    char* path;
    char* name;
    size_t size;
    int is_dir;
    int is_symlink;
    int is_binary;
    int is_hidden;
    time_t modified_time;
} cv_file_info_t;

typedef struct cv_fs_options {
    int hidden_files;
    int follow_symlinks;
    size_t max_size;
    char** includes;
    size_t include_count;
    char** excludes;
    size_t exclude_count;
    int git_tracked;
    int git_modified;
    int git_untracked;
} cv_fs_options_t;

typedef void (*cv_file_callback)(cv_file_info_t* file, void* user_data);

cv_fs_options_t* cv_fs_options_create(void);
void cv_fs_options_destroy(cv_fs_options_t* options);
int cv_fs_traverse(const char* path, cv_fs_options_t* options, cv_file_callback callback, void* user_data);
cv_file_info_t* cv_file_info_create(const char* path);
void cv_file_info_destroy(cv_file_info_t* file);
int cv_is_binary_file(const char* path);
int cv_is_hidden_file(const char* path);
int cv_matches_glob(const char* path, const char* glob);
int cv_should_ignore_file(const char* path, cv_fs_options_t* options);
char* cv_normalize_path(const char* path);
char* cv_get_extension(const char* path);

#endif
