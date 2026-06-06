#ifndef CLEARVISION_UTIL_ERROR_H
#define CLEARVISION_UTIL_ERROR_H

typedef enum {
    CV_ERROR_NONE = 0,
    CV_ERROR_MEMORY,
    CV_ERROR_IO,
    CV_ERROR_INVALID_ARG,
    CV_ERROR_NOT_FOUND,
    CV_ERROR_PERMISSION,
    CV_ERROR_THREAD,
    CV_ERROR_REGEX,
    CV_ERROR_DATABASE,
    CV_ERROR_UNKNOWN
} cv_error_code_t;

typedef struct cv_error {
    cv_error_code_t code;
    char message[256];
} cv_error_t;

void cv_error_set(cv_error_t* err, cv_error_code_t code, const char* fmt, ...);
const char* cv_error_string(cv_error_code_t code);

#endif
