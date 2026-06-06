#include "tui.h"
#include "cli/cli.h"
#include "fs/fs.h"
#include "search/search.h"
#include "output/output.h"
#include "util/memory.h"
#include "util/string.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct tui_state {
    char* search_pattern;
    char** results;
    size_t result_count;
    size_t selected_index;
    int running;
} tui_state_t;

static tui_state_t* tui_state_create(void) {
    tui_state_t* state = cv_calloc(1, sizeof(tui_state_t));
    state->search_pattern = cv_strdup("");
    state->selected_index = 0;
    state->running = 1;
    return state;
}

static void tui_state_destroy(tui_state_t* state) {
    if (!state) {
        return;
    }
    cv_free(state->search_pattern);
    for (size_t i = 0; i < state->result_count; i++) {
        cv_free(state->results[i]);
    }
    cv_free(state->results);
    cv_free(state);
}

static void tui_draw_header(WINDOW* win, tui_state_t* state) {
    wattron(win, A_REVERSE);
    mvwprintw(win, 0, 0, "ClearVision TUI - Pattern: %s", state->search_pattern);
    wclrtoeol(win);
    wattroff(win, A_REVERSE);
}

static void tui_draw_results(WINDOW* win, tui_state_t* state) {
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    for (int i = 0; i < max_y - 2 && i < (int)state->result_count; i++) {
        if (i == (int)state->selected_index) {
            wattron(win, A_REVERSE);
        }
        mvwprintw(win, i + 1, 0, "%s", state->results[i]);
        wclrtoeol(win);
        if (i == (int)state->selected_index) {
            wattroff(win, A_REVERSE);
        }
    }
}

static void tui_draw_footer(WINDOW* win) {
    wattron(win, A_REVERSE);
    mvwprintw(win, LINES - 1, 0, "q: quit | n/p: navigate | /: search | e: open in editor");
    wclrtoeol(win);
    wattroff(win, A_REVERSE);
}

static void tui_handle_input(tui_state_t* state) {
    int ch = getch();

    switch (ch) {
        case 'q':
        case 27:
            state->running = 0;
            break;
        case 'n':
        case KEY_DOWN:
            if (state->selected_index < state->result_count - 1) {
                state->selected_index++;
            }
            break;
        case 'p':
        case KEY_UP:
            if (state->selected_index > 0) {
                state->selected_index--;
            }
            break;
        case '/': {
            echo();
            mvprintw(LINES - 2, 0, "Search: ");
            char input[256];
            getnstr(input, sizeof(input) - 1);
            noecho();
            
            cv_free(state->search_pattern);
            state->search_pattern = cv_strdup(input);
            break;
        }
        case 'e':
            if (state->result_count > 0 && state->results[state->selected_index]) {
                char cmd[512];
                snprintf(cmd, sizeof(cmd), "vim %s", state->results[state->selected_index]);
                endwin();
                system(cmd);
                refresh();
            }
            break;
    }
}

int cv_tui_run(void) {
    tui_state_t* state = tui_state_create();

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    while (state->running) {
        erase();
        
        tui_draw_header(stdscr, state);
        tui_draw_results(stdscr, state);
        tui_draw_footer(stdscr);
        
        refresh();
        tui_handle_input(state);
    }

    endwin();
    tui_state_destroy(state);

    return 0;
}
