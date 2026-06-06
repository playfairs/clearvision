#include "output.h"
#include "../util/memory.h"
#include "../util/string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int color_enabled = 1;

void cv_output_set_color(int enabled) {
    color_enabled = enabled;
}

int cv_output_color_enabled(void) {
    return color_enabled;
}

static const char* color_reset(void) {
    return color_enabled ? "\033[0m" : "";
}

static const char* color_green(void) {
    return color_enabled ? "\033[32m" : "";
}

static const char* color_yellow(void) {
    return color_enabled ? "\033[33m" : "";
}

static const char* color_blue(void) {
    return color_enabled ? "\033[34m" : "";
}

static const char* color_cyan(void) {
    return color_enabled ? "\033[36m" : "";
}

static const char* color_bold(void) {
    return color_enabled ? "\033[1m" : "";
}

void cv_output_print_match(cv_match_t* match, cv_output_format_t format) {
    if (!match) {
        return;
    }

    switch (format) {
        case CV_OUTPUT_TERMINAL:
            printf("%s%s%s:%s%zu%s:%s%zu%s:%s",
                   color_cyan(), match->file_path, color_reset(),
                   color_green(), match->line_number, color_reset(),
                   color_yellow(), match->column, color_reset(),
                   color_bold());
            printf("%s%s%s\n", match->line_content, color_reset(), "");

            for (size_t i = 0; i < match->context_before_count; i++) {
                printf("%s%s\n", color_reset(), match->context_before[i]);
            }
            for (size_t i = 0; i < match->context_after_count; i++) {
                printf("%s%s\n", color_reset(), match->context_after[i]);
            }
            fflush(stdout);
            break;

        case CV_OUTPUT_JSON:
            printf("{\"file\":\"%s\",\"line\":%zu,\"column\":%zu,\"content\":\"%s\"}\n",
                   match->file_path, match->line_number, match->column, match->line_content);
            fflush(stdout);
            break;

        case CV_OUTPUT_TREE:
            printf("├── %s:%zu\n", match->file_path, match->line_number);
            fflush(stdout);
            break;

        case CV_OUTPUT_STATS:
            break;
    }
}

void cv_output_print_results(cv_search_result_t* result, cv_output_format_t format) {
    if (!result) {
        return;
    }

    if (format == CV_OUTPUT_JSON) {
        printf("[\n");
        for (size_t i = 0; i < result->match_count; i++) {
            if (i > 0) {
                printf(",\n");
            }
            cv_output_print_match(&result->matches[i], format);
        }
        printf("\n]\n");
    } else {
        for (size_t i = 0; i < result->match_count; i++) {
            cv_output_print_match(&result->matches[i], format);
        }
    }
    fflush(stdout);
}

void cv_output_print_stats(cv_search_result_t* result, uint64_t duration_ms) {
    if (!result) {
        return;
    }

    printf("\n%sStatistics%s\n", color_bold(), color_reset());
    printf("  Matches: %s%zu%s\n", color_green(), result->total_matches, color_reset());
    printf("  Files: %s%zu%s\n", color_blue(), result->file_count, color_reset());
    printf("  Duration: %s%llu%sms\n", color_yellow(), (unsigned long long)duration_ms, color_reset());
    fflush(stdout);
}
