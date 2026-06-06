#ifndef CLEARVISION_UTIL_UTIL_H
#define CLEARVISION_UTIL_UTIL_H

#include <stddef.h>
#include <stdint.h>

uint64_t cv_time_now_ms(void);
size_t cv_cpu_count(void);
int cv_is_terminal(void);
char* cv_get_home_dir(void);
char* cv_get_config_dir(void);
char* cv_get_cache_dir(void);
char* cv_get_data_dir(void);

#endif
