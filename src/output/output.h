#ifndef CLEARVISION_OUTPUT_OUTPUT_H
#define CLEARVISION_OUTPUT_OUTPUT_H

#include "../search/search.h"
#include <stddef.h>
#include <stdint.h>

typedef enum {
    CV_OUTPUT_TERMINAL,
    CV_OUTPUT_JSON,
    CV_OUTPUT_TREE,
    CV_OUTPUT_STATS
} cv_output_format_t;

void cv_output_print_match(cv_match_t* match, cv_output_format_t format);
void cv_output_print_results(cv_search_result_t* result, cv_output_format_t format);
void cv_output_print_stats(cv_search_result_t* result, uint64_t duration_ms);
void cv_output_set_color(int enabled);
int cv_output_color_enabled(void);

#endif
