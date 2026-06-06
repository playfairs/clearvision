#include "cli.h"
#include "../util/memory.h"
#include "../util/string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

cv_config_t* cv_config_create(void) {
    cv_config_t* config = cv_calloc(1, sizeof(cv_config_t));
    config->command = CV_COMMAND_NONE;
    config->context_before = 0;
    config->context_after = 0;
    config->context_around = 0;
    config->threads = 0;
    config->max_size = 0;
    config->git_tracked = 1;
    config->git_modified = 1;
    config->git_untracked = 0;
    return config;
}

void cv_config_destroy(cv_config_t* config) {
    if (!config) {
        return;
    }
    cv_free(config->pattern);
    for (size_t i = 0; i < config->path_count; i++) {
        cv_free(config->paths[i]);
    }
    cv_free(config->paths);
    for (size_t i = 0; i < config->include_count; i++) {
        cv_free(config->includes[i]);
    }
    cv_free(config->includes);
    for (size_t i = 0; i < config->exclude_count; i++) {
        cv_free(config->excludes[i]);
    }
    cv_free(config->excludes);
    cv_free(config);
}

static int is_flag(const char* arg) {
    return arg && arg[0] == '-';
}

static int parse_long_flag(const char* arg, cv_config_t* config) {
    if (strcmp(arg, "--json") == 0) {
        config->json_output = 1;
        return 1;
    }
    if (strcmp(arg, "--tree") == 0) {
        config->tree_output = 1;
        return 1;
    }
    if (strcmp(arg, "--stats") == 0) {
        config->stats_output = 1;
        return 1;
    }
    if (strcmp(arg, "--hidden") == 0) {
        config->hidden_files = 1;
        return 1;
    }
    if (strcmp(arg, "--follow") == 0) {
        config->follow_symlinks = 1;
        return 1;
    }
    return 0;
}

static int parse_short_flag(const char* arg, cv_config_t* config, int* i, int argc, char** argv) {
    if (strcmp(arg, "-i") == 0) {
        config->case_insensitive = 1;
        return 1;
    }
    if (strcmp(arg, "-w") == 0) {
        config->whole_word = 1;
        return 1;
    }
    if (strcmp(arg, "-c") == 0) {
        config->count_only = 1;
        return 1;
    }
    if (strcmp(arg, "-A") == 0) {
        if (*i + 1 < argc) {
            config->context_after = atoi(argv[++*i]);
        }
        return 1;
    }
    if (strcmp(arg, "-B") == 0) {
        if (*i + 1 < argc) {
            config->context_before = atoi(argv[++*i]);
        }
        return 1;
    }
    if (strcmp(arg, "-C") == 0) {
        if (*i + 1 < argc) {
            config->context_around = atoi(argv[++*i]);
        }
        return 1;
    }
    if (strcmp(arg, "-j") == 0) {
        if (*i + 1 < argc) {
            config->threads = (size_t)atoi(argv[++*i]);
        }
        return 1;
    }
    return 0;
}

int cv_config_parse(int argc, char** argv, cv_config_t* config) {
    if (argc < 2) {
        config->command = CV_COMMAND_HELP;
        return 0;
    }

    const char* first_arg = argv[1];
    
    if (strcmp(first_arg, "search") == 0) {
        config->command = CV_COMMAND_SEARCH;
    } else if (strcmp(first_arg, "regex") == 0) {
        config->command = CV_COMMAND_REGEX;
    } else if (strcmp(first_arg, "fuzzy") == 0) {
        config->command = CV_COMMAND_FUZZY;
    } else if (strcmp(first_arg, "index") == 0) {
        config->command = CV_COMMAND_INDEX;
    } else if (strcmp(first_arg, "rebuild") == 0) {
        config->command = CV_COMMAND_REBUILD;
    } else if (strcmp(first_arg, "remove-index") == 0) {
        config->command = CV_COMMAND_REMOVE_INDEX;
    } else if (strcmp(first_arg, "stats") == 0) {
        config->command = CV_COMMAND_STATS;
    } else if (strcmp(first_arg, "tui") == 0) {
        config->command = CV_COMMAND_TUI;
    } else if (strcmp(first_arg, "version") == 0) {
        config->command = CV_COMMAND_VERSION;
    } else if (strcmp(first_arg, "help") == 0) {
        config->command = CV_COMMAND_HELP;
    } else if (is_flag(first_arg)) {
        config->command = CV_COMMAND_SEARCH;
        config->pattern = cv_strdup(first_arg);
    } else {
        config->command = CV_COMMAND_SEARCH;
        config->pattern = cv_strdup(first_arg);
    }

    for (int i = 2; i < argc; i++) {
        const char* arg = argv[i];
        
        if (is_flag(arg)) {
            if (arg[1] == '-') {
                if (!parse_long_flag(arg, config)) {
                    return -1;
                }
            } else {
                if (!parse_short_flag(arg, config, &i, argc, argv)) {
                    return -1;
                }
            }
        } else if (config->command == CV_COMMAND_INDEX) {
            size_t new_count = config->path_count + 1;
            config->paths = cv_realloc(config->paths, new_count * sizeof(char*));
            config->paths[config->path_count] = cv_strdup(arg);
            config->path_count = new_count;
        } else if (!config->pattern) {
            config->pattern = cv_strdup(arg);
        } else {
            size_t new_count = config->path_count + 1;
            config->paths = cv_realloc(config->paths, new_count * sizeof(char*));
            config->paths[config->path_count] = cv_strdup(arg);
            config->path_count = new_count;
        }
    }

    if (config->path_count == 0) {
        config->paths = cv_malloc(sizeof(char*));
        config->paths[0] = cv_strdup(".");
        config->path_count = 1;
    }

    if (config->context_around > 0) {
        config->context_before = config->context_around;
        config->context_after = config->context_around;
    }

    if (config->threads == 0) {
        config->threads = 4;
    }

    return 0;
}

void cv_print_help(void) {
    printf("ClearVision\n");
    printf("Fast recursive search utility written in C\n\n");
    printf("Usage:\n");
    printf("  cv [command] [options]\n\n");
    printf("Commands:\n");
    printf("  search <pattern>    Literal string search\n");
    printf("  regex <pattern>     Regular expression search\n");
    printf("  fuzzy <pattern>     Fuzzy search\n");
    printf("  index <path>        Build search index\n");
    printf("  rebuild             Rebuild search index\n");
    printf("  remove-index        Remove search index\n");
    printf("  stats               Show index statistics\n");
    printf("  tui                 Interactive terminal interface\n");
    printf("  version             Show version information\n");
    printf("  help                Show this help message\n\n");
    printf("Options:\n");
    printf("  -i                  Case-insensitive search\n");
    printf("  -w                  Whole-word matching\n");
    printf("  -c                  Count matches only\n");
    printf("  -A <lines>          Context lines after match\n");
    printf("  -B <lines>          Context lines before match\n");
    printf("  -C <lines>          Context lines around match\n");
    printf("  -j <threads>        Number of threads\n");
    printf("  --json              JSON output format\n");
    printf("  --tree              Tree output format\n");
    printf("  --stats             Statistics output\n");
    printf("  --hidden            Include hidden files\n");
    printf("  --follow            Follow symbolic links\n");
    printf("  --include <glob>    Include files matching pattern\n");
    printf("  --exclude <glob>    Exclude files matching pattern\n");
    printf("  --max-size <bytes>  Maximum file size\n");
}

void cv_print_version(void) {
    printf("ClearVision 0.1.0\n");
}
