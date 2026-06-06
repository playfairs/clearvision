#include "search.h"
#include "../util/memory.h"
#include "../util/string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

cv_search_options_t* cv_search_options_create(void) {
    return cv_calloc(1, sizeof(cv_search_options_t));
}

void cv_search_options_destroy(cv_search_options_t* options) {
    if (!options) {
        return;
    }
    cv_free(options->pattern);
    cv_free(options);
}

cv_search_result_t* cv_search_result_create(void) {
    cv_search_result_t* result = cv_calloc(1, sizeof(cv_search_result_t));
    result->matches = cv_malloc(256 * sizeof(cv_match_t));
    return result;
}

void cv_search_result_destroy(cv_search_result_t* result) {
    if (!result) {
        return;
    }
    for (size_t i = 0; i < result->match_count; i++) {
        cv_match_t* match = &result->matches[i];
        cv_free(match->file_path);
        cv_free(match->line_content);
        for (size_t j = 0; j < match->context_before_count; j++) {
            cv_free(match->context_before[j]);
        }
        cv_free(match->context_before);
        for (size_t j = 0; j < match->context_after_count; j++) {
            cv_free(match->context_after[j]);
        }
        cv_free(match->context_after);
    }
    cv_free(result->matches);
    cv_free(result);
}

int cv_search_literal(const char* content, const char* pattern, int case_insensitive, int whole_word) {
    (void)content;
    (void)pattern;
    (void)case_insensitive;
    (void)whole_word;
    return 0;
}

int cv_search_regex(const char* content, const char* pattern, int case_insensitive) {
    (void)content;
    (void)pattern;
    (void)case_insensitive;
    return 0;
}

int cv_search_fuzzy(const char* content, const char* pattern, int case_insensitive) {
    (void)content;
    (void)pattern;
    (void)case_insensitive;
    return 0;
}

int cv_search_file(const char* path, cv_search_options_t* options, cv_search_result_t* result) {
    (void)path;
    (void)options;
    (void)result;
    return 0;
}
