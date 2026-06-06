#include "cli/cli.h"
#include "fs/fs.h"
#include "search/search.h"
#include "search/index.h"
#include "output/output.h"
#include "util/util.h"
#include "util/thread.h"
#include "util/memory.h"
#include "tui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int run_search(cv_config_t* config) {
    if (!config || !config->pattern) {
        return -1;
    }

    cv_search_options_t* search_options = cv_search_options_create();
    search_options->pattern = cv_strdup(config->pattern);
    search_options->case_insensitive = config->case_insensitive;
    search_options->whole_word = config->whole_word;
    search_options->context_before = config->context_before;
    search_options->context_after = config->context_after;
    search_options->threads = config->threads;

    switch (config->command) {
        case CV_COMMAND_SEARCH:
            search_options->type = CV_SEARCH_LITERAL;
            break;
        case CV_COMMAND_REGEX:
            search_options->type = CV_SEARCH_REGEX;
            break;
        case CV_COMMAND_FUZZY:
            search_options->type = CV_SEARCH_FUZZY;
            break;
        default:
            search_options->type = CV_SEARCH_LITERAL;
            break;
    }

    cv_search_result_t* result = cv_search_result_create();

    cv_fs_options_t* fs_options = cv_fs_options_create();
    fs_options->hidden_files = config->hidden_files;
    fs_options->follow_symlinks = config->follow_symlinks;
    fs_options->max_size = config->max_size;

    for (size_t i = 0; i < config->path_count; i++) {
        cv_fs_traverse(config->paths[i], fs_options, NULL, NULL);
    }

    for (size_t i = 0; i < config->path_count; i++) {
        cv_search_file(config->paths[i], search_options, result);
    }

    cv_output_format_t format = CV_OUTPUT_TERMINAL;
    if (config->json_output) {
        format = CV_OUTPUT_JSON;
    } else if (config->tree_output) {
        format = CV_OUTPUT_TREE;
    }

    cv_output_print_results(result, format);

    if (config->stats_output) {
        cv_output_print_stats(result, 0);
    }

    cv_search_options_destroy(search_options);
    cv_search_result_destroy(result);
    cv_fs_options_destroy(fs_options);

    return 0;
}

static int run_index(cv_config_t* config) {
    if (!config || config->path_count == 0) {
        return -1;
    }

    cv_index_t* index = cv_index_create(config->paths[0]);
    if (!index) {
        return -1;
    }

    int result = cv_index_build(index);
    cv_index_destroy(index);

    return result;
}

static int run_reindex(cv_config_t* config) {
    if (!config || config->path_count == 0) {
        return -1;
    }

    cv_index_t* index = cv_index_create(config->paths[0]);
    if (!index) {
        return -1;
    }

    int result = cv_index_rebuild(index);
    cv_index_destroy(index);

    return result;
}

static int run_remove_index(cv_config_t* config) {
    if (!config || config->path_count == 0) {
        return -1;
    }

    cv_index_t* index = cv_index_create(config->paths[0]);
    if (!index) {
        return -1;
    }

    int result = cv_index_remove(index);
    cv_index_destroy(index);

    return result;
}

static int run_stats(cv_config_t* config) {
    if (!config || config->path_count == 0) {
        return -1;
    }

    cv_index_t* index = cv_index_create(config->paths[0]);
    if (!index) {
        return -1;
    }

    char stats[1024];
    if (cv_index_get_stats(index, stats, sizeof(stats)) == 0) {
        printf("%s\n", stats);
    }

    cv_index_destroy(index);

    return 0;
}

static int run_tui(cv_config_t* config) {
    (void)config;
    return cv_tui_run();
}

int main(int argc, char** argv) {
    cv_config_t* config = cv_config_create();
    
    if (cv_config_parse(argc, argv, config) != 0) {
        cv_config_destroy(config);
        return 1;
    }

    int result = 0;

    switch (config->command) {
        case CV_COMMAND_SEARCH:
        case CV_COMMAND_REGEX:
        case CV_COMMAND_FUZZY:
            result = run_search(config);
            break;
        case CV_COMMAND_INDEX:
            result = run_index(config);
            break;
        case CV_COMMAND_REBUILD:
            result = run_reindex(config);
            break;
        case CV_COMMAND_REMOVE_INDEX:
            result = run_remove_index(config);
            break;
        case CV_COMMAND_STATS:
            result = run_stats(config);
            break;
        case CV_COMMAND_TUI:
            result = run_tui(config);
            break;
        case CV_COMMAND_VERSION:
            cv_print_version();
            break;
        case CV_COMMAND_HELP:
            cv_print_help();
            break;
        default:
            cv_print_help();
            break;
    }

    cv_config_destroy(config);
    return result;
}
