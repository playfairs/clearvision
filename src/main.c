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

    char cmd[4096];
    size_t cmd_len = 0;

    cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, "rg");

    if (config->case_insensitive) {
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, " -i");
    }

    if (config->whole_word) {
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, " -w");
    }

    if (config->context_before > 0) {
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, " -B %d", config->context_before);
    }

    if (config->context_after > 0) {
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, " -A %d", config->context_after);
    }

    if (config->context_around > 0) {
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, " -C %d", config->context_around);
    }

    if (config->hidden_files) {
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, " --hidden");
    }

    if (config->follow_symlinks) {
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, " --follow");
    }

    cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, " --line-number --color ansi --heading");

    cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, " %s", config->pattern);

    for (size_t i = 0; i < config->path_count; i++) {
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, " %s", config->paths[i]);
    }

    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        fprintf(stderr, "Error: Failed to execute ripgrep. Is ripgrep installed?\n");
        fflush(stderr);
        return -1;
    }

    char buffer[8192];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        printf("%s", buffer);
        fflush(stdout);
    }

    int status = pclose(pipe);
    if (status != 0) {
        fprintf(stderr, "Error: ripgrep exited with status %d\n", status);
        fflush(stderr);
    }

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
