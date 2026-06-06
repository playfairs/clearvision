#include "error.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void cv_error_set(cv_error_t* err, cv_error_code_t code, const char* fmt, ...) {
    if (!err) {
        return;
    }

    err->code = code;

    if (fmt) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(err->message, sizeof(err->message), fmt, args);
        va_end(args);
    } else {
        snprintf(err->message, sizeof(err->message), "%s", cv_error_string(code));
    }
}

const char* cv_error_string(cv_error_code_t code) {
    switch (code) {
        case CV_ERROR_NONE: return "No error";
        case CV_ERROR_MEMORY: return "Memory allocation failed";
        case CV_ERROR_IO: return "I/O error";
        case CV_ERROR_INVALID_ARG: return "Invalid argument";
        case CV_ERROR_NOT_FOUND: return "Not found";
        case CV_ERROR_PERMISSION: return "Permission denied";
        case CV_ERROR_THREAD: return "Thread error";
        case CV_ERROR_REGEX: return "Regular expression error";
        case CV_ERROR_DATABASE: return "Database error";
        case CV_ERROR_UNKNOWN: return "Unknown error";
        default: return "Invalid error code";
    }
}
