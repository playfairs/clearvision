#include "fs.h"
#include "../util/memory.h"
#include "../util/string.h"
#include <dirent.h>
#include <fnmatch.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

cv_fs_options_t* cv_fs_options_create(void) {
    return cv_calloc(1, sizeof(cv_fs_options_t));
}

void cv_fs_options_destroy(cv_fs_options_t* options) {
    if (!options) {
        return;
    }
    for (size_t i = 0; i < options->include_count; i++) {
        cv_free(options->includes[i]);
    }
    cv_free(options->includes);
    for (size_t i = 0; i < options->exclude_count; i++) {
        cv_free(options->excludes[i]);
    }
    cv_free(options->excludes);
    cv_free(options);
}

cv_file_info_t* cv_file_info_create(const char* path) {
    if (!path) {
        return NULL;
    }

    cv_file_info_t* file = cv_calloc(1, sizeof(cv_file_info_t));
    file->path = cv_strdup(path);

    char* last_slash = strrchr(path, '/');
    file->name = last_slash ? cv_strdup(last_slash + 1) : cv_strdup(path);

    struct stat st;
    if (stat(path, &st) == 0) {
        file->size = (size_t)st.st_size;
        file->is_dir = S_ISDIR(st.st_mode);
        file->is_symlink = S_ISLNK(st.st_mode);
        file->modified_time = st.st_mtime;
        file->is_binary = cv_is_binary_file(path);
        file->is_hidden = cv_is_hidden_file(path);
    }

    return file;
}

void cv_file_info_destroy(cv_file_info_t* file) {
    if (!file) {
        return;
    }
    cv_free(file->path);
    cv_free(file->name);
    cv_free(file);
}

int cv_is_binary_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        return 0;
    }

    unsigned char buffer[8192];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer), file);
    fclose(file);

    if (bytes_read == 0) {
        return 0;
    }

    size_t null_count = 0;
    for (size_t i = 0; i < bytes_read; i++) {
        if (buffer[i] == 0) {
            null_count++;
        }
    }

    if (null_count > bytes_read / 32) {
        return 1;
    }

    for (size_t i = 0; i < bytes_read; i++) {
        if (buffer[i] < 9 || (buffer[i] > 13 && buffer[i] < 32)) {
            return 1;
        }
    }

    return 0;
}

int cv_is_hidden_file(const char* path) {
    const char* name = strrchr(path, '/');
    name = name ? name + 1 : path;
    return name && name[0] == '.';
}

int cv_matches_glob(const char* path, const char* glob) {
    if (!path || !glob) {
        return 0;
    }
    const char* name = strrchr(path, '/');
    name = name ? name + 1 : path;
    return fnmatch(glob, name, FNM_PATHNAME) == 0;
}

int cv_should_ignore_file(const char* path, cv_fs_options_t* options) {
    if (!options) {
        return 0;
    }

    if (!options->hidden_files && cv_is_hidden_file(path)) {
        return 1;
    }

    if (options->include_count > 0) {
        int included = 0;
        for (size_t i = 0; i < options->include_count; i++) {
            if (cv_matches_glob(path, options->includes[i])) {
                included = 1;
                break;
            }
        }
        if (!included) {
            return 1;
        }
    }

    for (size_t i = 0; i < options->exclude_count; i++) {
        if (cv_matches_glob(path, options->excludes[i])) {
            return 1;
        }
    }

    return 0;
}

char* cv_normalize_path(const char* path) {
    if (!path) {
        return NULL;
    }

    char* resolved = cv_malloc(PATH_MAX);
    if (!realpath(path, resolved)) {
        cv_free(resolved);
        return cv_strdup(path);
    }

    return resolved;
}

char* cv_get_extension(const char* path) {
    if (!path) {
        return NULL;
    }

    const char* last_dot = strrchr(path, '.');
    const char* last_slash = strrchr(path, '/');

    if (last_dot && (!last_slash || last_dot > last_slash)) {
        return cv_strdup(last_dot + 1);
    }

    return NULL;
}

static int traverse_directory(const char* path, cv_fs_options_t* options, cv_file_callback callback, void* user_data) {
    DIR* dir = opendir(path);
    if (!dir) {
        return -1;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char* full_path = cv_malloc(strlen(path) + strlen(entry->d_name) + 2);
        sprintf(full_path, "%s/%s", path, entry->d_name);

        cv_file_info_t* file = cv_file_info_create(full_path);

        if (cv_should_ignore_file(full_path, options)) {
            cv_file_info_destroy(file);
            cv_free(full_path);
            continue;
        }

        if (callback) {
            callback(file, user_data);
        }

        if (file->is_dir && (options->follow_symlinks || !file->is_symlink)) {
            traverse_directory(full_path, options, callback, user_data);
        }

        cv_file_info_destroy(file);
        cv_free(full_path);
    }

    closedir(dir);
    return 0;
}

int cv_fs_traverse(const char* path, cv_fs_options_t* options, cv_file_callback callback, void* user_data) {
    if (!path) {
        return -1;
    }

    cv_file_info_t* file = cv_file_info_create(path);
    if (!file) {
        return -1;
    }

    if (callback) {
        callback(file, user_data);
    }

    if (file->is_dir && (options->follow_symlinks || !file->is_symlink)) {
        traverse_directory(path, options, callback, user_data);
    }

    cv_file_info_destroy(file);
    return 0;
}
