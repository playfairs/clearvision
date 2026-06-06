#define PCRE2_CODE_UNIT_WIDTH 8
#include "search.h"
#include "../util/memory.h"
#include "../util/string.h"
#include <pcre2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

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
    return cv_calloc(1, sizeof(cv_search_result_t));
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

static int is_word_boundary(char c) {
    return c == '\0' || c == ' ' || c == '\t' || c == '\n' || c == '\r' || 
           c == '.' || c == ',' || c == ';' || c == ':' || c == '!' || 
           c == '?' || c == '(' || c == ')' || c == '[' || c == ']' || 
           c == '{' || c == '}' || c == '<' || c == '>' || c == '/' || 
           c == '\\' || c == '|' || c == '-' || c == '_' || c == '+' || 
           c == '=' || c == '*' || c == '&' || c == '^' || c == '%' || 
           c == '$' || c == '#' || c == '@' || c == '~' || c == '`';
}

int cv_search_literal(const char* content, const char* pattern, int case_insensitive, int whole_word) {
    if (!content || !pattern) {
        return 0;
    }

    size_t content_len = strlen(content);
    size_t pattern_len = strlen(pattern);

    if (pattern_len == 0 || pattern_len > content_len) {
        return 0;
    }

    for (size_t i = 0; i <= content_len - pattern_len; i++) {
        int match = 0;
        
        if (case_insensitive) {
            match = strncasecmp(content + i, pattern, pattern_len) == 0;
        } else {
            match = strncmp(content + i, pattern, pattern_len) == 0;
        }

        if (match) {
            if (whole_word) {
                char before = (i > 0) ? content[i - 1] : '\0';
                char after = content[i + pattern_len];
                
                if (is_word_boundary(before) && is_word_boundary(after)) {
                    return 1;
                }
            } else {
                return 1;
            }
        }
    }

    return 0;
}

int cv_search_regex(const char* content, const char* pattern, int case_insensitive) {
    if (!content || !pattern) {
        return 0;
    }

    int error_code;
    PCRE2_SIZE error_offset;
    
    uint32_t options = PCRE2_UTF;
    if (case_insensitive) {
        options |= PCRE2_CASELESS;
    }

    pcre2_code* regex = pcre2_compile(
        (PCRE2_SPTR)pattern,
        PCRE2_ZERO_TERMINATED,
        options,
        &error_code,
        &error_offset,
        NULL
    );

    if (!regex) {
        return 0;
    }

    pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(regex, NULL);
    
    int rc = pcre2_match(
        regex,
        (PCRE2_SPTR)content,
        PCRE2_ZERO_TERMINATED,
        0,
        0,
        match_data,
        NULL
    );

    pcre2_match_data_free(match_data);
    pcre2_code_free(regex);

    return rc >= 0;
}

static size_t levenshtein_distance(const char* s1, const char* s2, int case_insensitive) {
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    
    if (len1 == 0) return len2;
    if (len2 == 0) return len1;

    size_t* prev = cv_malloc((len2 + 1) * sizeof(size_t));
    size_t* curr = cv_malloc((len2 + 1) * sizeof(size_t));

    for (size_t i = 0; i <= len2; i++) {
        prev[i] = i;
    }

    for (size_t i = 1; i <= len1; i++) {
        curr[0] = i;
        for (size_t j = 1; j <= len2; j++) {
            char c1 = case_insensitive ? tolower((unsigned char)s1[i - 1]) : s1[i - 1];
            char c2 = case_insensitive ? tolower((unsigned char)s2[j - 1]) : s2[j - 1];
            
            size_t cost = (c1 == c2) ? 0 : 1;
            curr[j] = prev[j] + 1;
            size_t temp = curr[j - 1] + 1;
            if (temp < curr[j]) curr[j] = temp;
            temp = prev[j - 1] + cost;
            if (temp < curr[j]) curr[j] = temp;
        }
        size_t* temp = prev;
        prev = curr;
        curr = temp;
    }

    size_t result = prev[len2];
    cv_free(prev);
    cv_free(curr);
    return result;
}

int cv_search_fuzzy(const char* content, const char* pattern, int case_insensitive) {
    if (!content || !pattern) {
        return 0;
    }

    size_t pattern_len = strlen(pattern);
    if (pattern_len == 0) {
        return 0;
    }

    size_t content_len = strlen(content);
    size_t window_size = pattern_len + (pattern_len / 2);
    
    if (window_size > content_len) {
        window_size = content_len;
    }

    for (size_t i = 0; i <= content_len - window_size; i++) {
        char* window = cv_malloc(window_size + 1);
        memcpy(window, content + i, window_size);
        window[window_size] = '\0';

        size_t distance = levenshtein_distance(window, pattern, case_insensitive);
        cv_free(window);

        if (distance <= (pattern_len / 3)) {
            return 1;
        }
    }

    return 0;
}

static char* read_file_content(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = cv_malloc(size + 1);
    fread(content, 1, size, file);
    content[size] = '\0';

    fclose(file);
    return content;
}

static char** get_lines(const char* content, size_t* line_count) {
    if (!content || !line_count) {
        return NULL;
    }

    size_t capacity = 256;
    char** lines = cv_malloc(capacity * sizeof(char*));
    *line_count = 0;

    const char* start = content;
    while (*start) {
        const char* end = start;
        while (*end && *end != '\n') {
            end++;
        }

        size_t len = end - start;
        char* line = cv_malloc(len + 1);
        memcpy(line, start, len);
        line[len] = '\0';

        if (*line_count >= capacity) {
            capacity *= 2;
            lines = cv_realloc(lines, capacity * sizeof(char*));
        }
        lines[*line_count] = line;
        (*line_count)++;

        if (*end) {
            start = end + 1;
        } else {
            break;
        }
    }

    return lines;
}

static void free_lines(char** lines, size_t count) {
    if (!lines) {
        return;
    }
    for (size_t i = 0; i < count; i++) {
        cv_free(lines[i]);
    }
    cv_free(lines);
}

int cv_search_file(const char* path, cv_search_options_t* options, cv_search_result_t* result) {
    if (!path || !options || !result) {
        return -1;
    }

    char* content = read_file_content(path);
    if (!content) {
        return -1;
    }

    size_t line_count;
    char** lines = get_lines(content, &line_count);

    for (size_t i = 0; i < line_count; i++) {
        int match = 0;
        
        switch (options->type) {
            case CV_SEARCH_LITERAL:
                match = cv_search_literal(lines[i], options->pattern, options->case_insensitive, options->whole_word);
                break;
            case CV_SEARCH_REGEX:
                match = cv_search_regex(lines[i], options->pattern, options->case_insensitive);
                break;
            case CV_SEARCH_FUZZY:
                match = cv_search_fuzzy(lines[i], options->pattern, options->case_insensitive);
                break;
        }

        if (match) {
            size_t new_count = result->match_count + 1;
            result->matches = cv_realloc(result->matches, new_count * sizeof(cv_match_t));
            
            cv_match_t* match_entry = &result->matches[result->match_count];
            match_entry->file_path = cv_strdup(path);
            match_entry->line_number = i + 1;
            match_entry->column = 0;
            match_entry->match_start = 0;
            match_entry->match_end = 0;
            match_entry->line_content = cv_strdup(lines[i]);
            match_entry->context_before = NULL;
            match_entry->context_before_count = 0;
            match_entry->context_after = NULL;
            match_entry->context_after_count = 0;

            if (options->context_before > 0 && i > 0) {
                size_t ctx_start = (i > options->context_before) ? i - options->context_before : 0;
                match_entry->context_before_count = i - ctx_start;
                match_entry->context_before = cv_malloc(match_entry->context_before_count * sizeof(char*));
                for (size_t j = 0; j < match_entry->context_before_count; j++) {
                    match_entry->context_before[j] = cv_strdup(lines[ctx_start + j]);
                }
            }

            if (options->context_after > 0 && i < line_count - 1) {
                size_t ctx_end = (i + options->context_after < line_count) ? i + options->context_after : line_count - 1;
                match_entry->context_after_count = ctx_end - i;
                match_entry->context_after = cv_malloc(match_entry->context_after_count * sizeof(char*));
                for (size_t j = 0; j < match_entry->context_after_count; j++) {
                    match_entry->context_after[j] = cv_strdup(lines[i + 1 + j]);
                }
            }

            result->match_count = new_count;
            result->total_matches++;
        }
    }

    free_lines(lines, line_count);
    cv_free(content);

    if (result->match_count > 0) {
        result->file_count++;
    }

    return 0;
}
