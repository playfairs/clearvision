#ifndef CLEARVISION_SEARCH_SEARCH_H
#define CLEARVISION_SEARCH_SEARCH_H

#include <stddef.h>

typedef enum {
    CV_SEARCH_LITERAL,
    CV_SEARCH_REGEX,
    CV_SEARCH_FUZZY
} cv_search_type_t;

typedef struct cv_match {
    char* file_path;
    size_t line_number;
    size_t column;
    size_t match_start;
    size_t match_end;
    char* line_content;
    char** context_before;
    size_t context_before_count;
    char** context_after;
    size_t context_after_count;
} cv_match_t;

typedef struct cv_search_options {
    cv_search_type_t type;
    char* pattern;
    int case_insensitive;
    int whole_word;
    int exact_match;
    size_t context_before;
    size_t context_after;
    size_t threads;
} cv_search_options_t;

typedef struct cv_search_result {
    cv_match_t* matches;
    size_t match_count;
    size_t file_count;
    size_t total_matches;
} cv_search_result_t;

cv_search_options_t* cv_search_options_create(void);
void cv_search_options_destroy(cv_search_options_t* options);
cv_search_result_t* cv_search_result_create(void);
void cv_search_result_destroy(cv_search_result_t* result);
cv_search_result_t* cv_search_execute(const char* path, cv_search_options_t* options);
int cv_search_file(const char* path, cv_search_options_t* options, cv_search_result_t* result);
int cv_search_literal(const char* content, const char* pattern, int case_insensitive, int whole_word);
int cv_search_regex(const char* content, const char* pattern, int case_insensitive);
int cv_search_fuzzy(const char* content, const char* pattern, int case_insensitive);

#endif
