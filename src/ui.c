#define _XOPEN_SOURCE_EXTENDED 1
#include <ncursesw/curses.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include "ui.h"
#include "metrics.h"

static int win_rows, win_cols;

#define COL_LABEL 1
#define COL_LOW   2
#define COL_MID   3
#define COL_HIGH  4
#define COL_HEAD  5

static void draw_stat_line(int row, int col, const char* label, const char* value, int color_id, int total_width) {
    mvprintw(row, col, "%s: ", label);
    int label_padding = strlen(label) + 2; 
    int space_left = total_width - label_padding;

    if (space_left > 0) {
        attron(COLOR_PAIR(color_id) | A_BOLD);
        printw("%.*s", space_left, value);
        attroff(COLOR_PAIR(color_id) | A_BOLD);
    }
}

void draw_box(int y, int x, int h, int w, const char* title) {
    attron(A_BOLD);
    mvaddstr(y, x, U_TOP_LEFT);
    mvaddstr(y, x + w - 1, U_TOP_RIGHT);
    mvaddstr(y + h - 1, x, U_BOT_LEFT);
    mvaddstr(y + h - 1, x + w - 1, U_BOT_RIGHT);

    for (int i = 1; i < w - 1; i++) {
        mvaddstr(y, x + i, U_HORIZ);
        mvaddstr(y + h - 1, x + i, U_HORIZ);
    }
    for (int i = 1; i < h - 1; i++) {
        mvaddstr(y + i, x, U_VERT);
        mvaddstr(y + i, x + w - 1, U_VERT);
    }
    if (title) mvprintw(y, x + 2, " %s ", title);
    attroff(A_BOLD);
}

static void draw_bar(int y, int x, int w, double pct) {
    int filled = (int)((pct / 100.0) * w);
    int pair = (pct > 80) ? 4 : (pct > 50 ? 3 : 2);

    mvaddch(y, x, '[');
    attron(COLOR_PAIR(pair));
    for (int i = 0; i < filled; i++) mvaddch(y, x + 1 + i, ACS_CKBOARD);
    attroff(COLOR_PAIR(pair));

    for (int i = filled; i < w; i++) mvaddch(y, x + 1 + i, '.');
    mvaddch(y, x + w + 1, ']');
}

/* --- Section Rendering --- */

static int draw_cpu_section(int y, int x, int w, double load, const cpu_stats_t *cpu) {
    int h = 7; 
    draw_box(y, x, h, w, " CPU ");
    int pair = (load > 80) ? 4 : (load > 50 ? 3 : 2);
    char load_str[16];
    snprintf(load_str, sizeof(load_str), "%.2f%%", load);
    draw_stat_line(y + 1, x + 2, "Load", load_str, pair, w - 4);
    draw_bar(y + 2, x + 2, w - 6, load);

    char ticks[128];
    snprintf(ticks, sizeof(ticks), "us:%llu sy:%llu id:%llu", cpu->user, cpu->system, cpu->idle);
    draw_stat_line(y + 4, x + 2, "Ticks", ticks, COL_LABEL, w - 4);
    return h;
}

static int draw_mem_section(int y, int x, int w, const mem_stats_t *m) {
    int h = 7; 
    draw_box(y, x, h, w, " MEMORY ");
    double used = (double)(m->total_mem - m->avail_mem);
    double pct = (m->total_mem > 0) ? (100.0 * used / m->total_mem) : 0.0;
    char mem_str[32];
    snprintf(mem_str, sizeof(mem_str), "%.1fG/%.1fG", used/1024.0/1024.0, (double)m->total_mem/1024.0/1024.0);
    draw_stat_line(y + 1, x + 2, "Usage", mem_str, COL_LABEL, w - 4);
    draw_bar(y + 2, x + 2, w - 6, pct);
    
    char pct_label[16];
    snprintf(pct_label, sizeof(pct_label), "%.1f%%", pct);
    draw_stat_line(y + 4, x + 2, "Load", pct_label, COL_LABEL, w - 4);
    return h;
}

static int draw_net_section(int y, int w, const net_stats_t *n) {
    int h = 5;
    draw_box(y, 0, h, w, " NETWORK ");
    char rx[64], tx[64];
    snprintf(rx, sizeof(rx), "Total: %.2f MB | Rate: %.2f KB/s", n->rx_bytes/1024.0/1024.0, n->rx_rate);
    snprintf(tx, sizeof(tx), "Total: %.2f MB | Rate: %.2f KB/s", n->tx_bytes/1024.0/1024.0, n->tx_rate);
    draw_stat_line(y + 1, 2, "Down", rx, COL_LABEL, w - 4);
    draw_stat_line(y + 2, 2, "Up  ", tx, COL_LABEL, w - 4);
    return h;
}

static int draw_disk_section(int y, int w, const disk_stats_t *d) {
    int h = 5;
    draw_box(y, 0, h, w, " DISK ");
    char disk_str[64];
    snprintf(disk_str, sizeof(disk_str), "%.1fG/%.1fG (%1.f%%)", d->used, d->total, d->percentage);
    draw_stat_line(y + 1, 2, "Mount /", disk_str, COL_LABEL, w - 4);
    draw_bar(y + 2, 2, w - 6, d->percentage);
    return h;
}

void init_ui(void) {
    setlocale(LC_ALL, "");
    initscr(); noecho(); cbreak(); curs_set(FALSE);
    keypad(stdscr, TRUE);
    timeout(500);
    if (has_colors()) {
        start_color(); use_default_colors();
        init_pair(1, COLOR_CYAN, -1); 
        init_pair(2, COLOR_GREEN, -1); 
        init_pair(3, COLOR_YELLOW, -1); 
        init_pair(4, COLOR_RED, -1); 
        init_pair(5, COLOR_MAGENTA, -1); 
    }
}

void end_ui(void) { endwin(); }

void render_ui(const char *distro, const char *hostname, double uptime, 
               const char *time_buf, double cpu_load, const mem_stats_t *m, 
               const cpu_stats_t *cpu, const disk_stats_t *disk, 
               const net_stats_t *net, int active_tab) 
{
    getmaxyx(stdscr, win_rows, win_cols);
    erase();

    int box_w = (win_cols < 24) ? 20 : win_cols - 4;
    int half_w = (box_w / 2) - 1;
    int second_col_x = half_w + 2;
    int cur_y = 1;

    // Header
    attron(COLOR_PAIR(COL_HEAD) | A_BOLD);
    mvprintw(cur_y++, 1, " puroba.sys ");
    attroff(A_BOLD | COLOR_PAIR(COL_HEAD));
    mvprintw(cur_y - 1, box_w - 15, "[ %s ]", (active_tab == 0) ? "OVERVIEW" : "TESTING");
    cur_y++;


    // Tab Content 
    if (active_tab == 0) {
        // Static Info 
        draw_box(cur_y, 0, 3, half_w, " OS ");
        draw_stat_line(cur_y + 1, 2, "Dist", distro, COL_LABEL, half_w - 4);
        draw_box(cur_y, second_col_x, 3, half_w, " TIME ");
        draw_stat_line(cur_y + 1, second_col_x + 2, "Now", time_buf, COL_LABEL, half_w - 4);
        cur_y += 4;

        // CPU and Memory
        draw_cpu_section(cur_y, 0, half_w, cpu_load, cpu);
        draw_mem_section(cur_y, second_col_x, half_w, m);
        cur_y += 8; // Both sections are height 7 + 1 for padding

        cur_y += draw_disk_section(cur_y, box_w, disk) + 1;
        cur_y += draw_net_section(cur_y, box_w, net) + 1;

    } else {
        draw_stat_line(cur_y + 1, 2, "Testing", "Coming soon.", COL_LABEL, half_w - 4);

    }

    // Footer
    attron(A_DIM);
    mvprintw(win_rows - 2, 2, "Uptime: %02d:%02d:%02d", (int)(uptime/3600), ((int)uptime%3600)/60, (int)uptime%60);
    attroff(A_DIM);

    attron(COLOR_PAIR(COL_LABEL) | A_REVERSE);
    mvprintw(win_rows - 1, 0, " [1-2]: Tabs | Q: Exit | Host: %s | %dx%d ", hostname, win_cols, win_rows);
    for(int i = getcurx(stdscr); i < win_cols; i++) addch(' ');
    attroff(A_REVERSE);

    refresh();
}
