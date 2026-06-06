#ifndef CLEARVISION_UTIL_STRING_H
#define CLEARVISION_UTIL_STRING_H

#include <stddef.h>

char* cv_string_append(char* str, const char* append);
char* cv_string_append_char(char* str, char c);
char* cv_string_reserve(size_t capacity);
size_t cv_string_length(const char* str);
int cv_string_compare(const char* a, const char* b);
int cv_string_case_compare(const char* a, const char* b);
char* cv_string_to_lower(const char* str);
char* cv_string_to_upper(const char* str);
int cv_string_starts_with(const char* str, const char* prefix);
int cv_string_ends_with(const char* str, const char* suffix);
char* cv_string_trim(char* str);
char** cv_string_split(const char* str, char delimiter, size_t* count);
void cv_string_free_array(char** array, size_t count);

#endif
