#include "util.h"
#include "memory.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <sys/ioctl.h>

#if defined(__linux__)
#include <sys/sysinfo.h>
#elif defined(__FreeBSD__)
#include <sys/sysctl.h>
#endif

uint64_t cv_time_now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

size_t cv_cpu_count(void) {
#ifdef __APPLE__
    int count;
    size_t size = sizeof(count);
    sysctlbyname("hw.logicalcpu", &count, &size, NULL, 0);
    return (size_t)count;
#elif defined(__linux__)
    return (size_t)sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(__FreeBSD__)
    int count;
    size_t size = sizeof(count);
    sysctlbyname("hw.ncpu", &count, &size, NULL, 0);
    return (size_t)count;
#else
    return 1;
#endif
}

int cv_is_terminal(void) {
    return isatty(STDOUT_FILENO);
}

char* cv_get_home_dir(void) {
    char* home = getenv("HOME");
    if (home) {
        return cv_strdup(home);
    }
    return NULL;
}

char* cv_get_config_dir(void) {
    char* home = cv_get_home_dir();
    if (!home) {
        return NULL;
    }
    char* config = cv_string_append(home, "/.config/clearvision");
    cv_free(home);
    return config;
}

char* cv_get_cache_dir(void) {
    char* home = cv_get_home_dir();
    if (!home) {
        return NULL;
    }
    char* cache = cv_string_append(home, "/.cache/clearvision");
    cv_free(home);
    return cache;
}

char* cv_get_data_dir(void) {
    char* home = cv_get_home_dir();
    if (!home) {
        return NULL;
    }
    char* data = cv_string_append(home, "/.local/share/clearvision");
    cv_free(home);
    return data;
}
