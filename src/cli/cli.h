#ifndef CLEARVISION_CLI_CLI_H
#define CLEARVISION_CLI_CLI_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    CV_COMMAND_NONE,
    CV_COMMAND_SEARCH,
    CV_COMMAND_REGEX,
    CV_COMMAND_FUZZY,
    CV_COMMAND_INDEX,
    CV_COMMAND_REBUILD,
    CV_COMMAND_REMOVE_INDEX,
    CV_COMMAND_STATS,
    CV_COMMAND_TUI,
    CV_COMMAND_VERSION,
    CV_COMMAND_HELP
} cv_command_t;

typedef struct cv_config {
    cv_command_t command;
    char* pattern;
    char** paths;
    size_t path_count;
    char** includes;
    size_t include_count;
    char** excludes;
    size_t exclude_count;
    int case_insensitive;
    int whole_word;
    int count_only;
    int context_before;
    int context_after;
    int context_around;
    size_t threads;
    int json_output;
    int tree_output;
    int stats_output;
    int hidden_files;
    int follow_symlinks;
    size_t max_size;
    int git_tracked;
    int git_modified;
    int git_untracked;
} cv_config_t;

cv_config_t* cv_config_create(void);
void cv_config_destroy(cv_config_t* config);
int cv_config_parse(int argc, char** argv, cv_config_t* config);
void cv_print_help(void);
void cv_print_version(void);

#endif
