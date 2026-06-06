#include "string.h"
#include "memory.h"
#include <ctype.h>
#include <string.h>

char* cv_string_append(char* str, const char* append) {
    if (!append) {
        return str;
    }
    size_t str_len = str ? strlen(str) : 0;
    size_t append_len = strlen(append);
    char* result = cv_malloc(str_len + append_len + 1);
    if (str) {
        memcpy(result, str, str_len);
        cv_free(str);
    }
    memcpy(result + str_len, append, append_len + 1);
    return result;
}

char* cv_string_append_char(char* str, char c) {
    size_t str_len = str ? strlen(str) : 0;
    char* result = cv_malloc(str_len + 2);
    if (str) {
        memcpy(result, str, str_len);
        cv_free(str);
    }
    result[str_len] = c;
    result[str_len + 1] = '\0';
    return result;
}

char* cv_string_reserve(size_t capacity) {
    char* str = cv_malloc(capacity + 1);
    str[0] = '\0';
    return str;
}

size_t cv_string_length(const char* str) {
    return str ? strlen(str) : 0;
}

int cv_string_compare(const char* a, const char* b) {
    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return 1;
    return strcmp(a, b);
}

int cv_string_case_compare(const char* a, const char* b) {
    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return 1;
    return strcasecmp(a, b);
}

char* cv_string_to_lower(const char* str) {
    if (!str) {
        return NULL;
    }
    size_t len = strlen(str);
    char* result = cv_malloc(len + 1);
    for (size_t i = 0; i < len; i++) {
        result[i] = tolower((unsigned char)str[i]);
    }
    result[len] = '\0';
    return result;
}

char* cv_string_to_upper(const char* str) {
    if (!str) {
        return NULL;
    }
    size_t len = strlen(str);
    char* result = cv_malloc(len + 1);
    for (size_t i = 0; i < len; i++) {
        result[i] = toupper((unsigned char)str[i]);
    }
    result[len] = '\0';
    return result;
}

int cv_string_starts_with(const char* str, const char* prefix) {
    if (!str || !prefix) {
        return 0;
    }
    size_t str_len = strlen(str);
    size_t prefix_len = strlen(prefix);
    if (prefix_len > str_len) {
        return 0;
    }
    return strncmp(str, prefix, prefix_len) == 0;
}

int cv_string_ends_with(const char* str, const char* suffix) {
    if (!str || !suffix) {
        return 0;
    }
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) {
        return 0;
    }
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

char* cv_string_trim(char* str) {
    if (!str) {
        return NULL;
    }
    char* start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    char* end = str + strlen(str) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
    if (start != str) {
        memmove(str, start, end - start + 2);
    }
    return str;
}

char** cv_string_split(const char* str, char delimiter, size_t* count) {
    if (!str || !count) {
        return NULL;
    }
    size_t capacity = 16;
    char** result = cv_malloc(capacity * sizeof(char*));
    *count = 0;
    
    const char* start = str;
    while (*start) {
        const char* end = start;
        while (*end && *end != delimiter) {
            end++;
        }
        size_t len = end - start;
        char* token = cv_malloc(len + 1);
        memcpy(token, start, len);
        token[len] = '\0';
        
        if (*count >= capacity) {
            capacity *= 2;
            result = cv_realloc(result, capacity * sizeof(char*));
        }
        result[*count] = token;
        (*count)++;
        
        if (*end) {
            start = end + 1;
        } else {
            break;
        }
    }
    
    return result;
}

void cv_string_free_array(char** array, size_t count) {
    if (!array) {
        return;
    }
    for (size_t i = 0; i < count; i++) {
        cv_free(array[i]);
    }
    cv_free(array);
}
