#define _XOPEN_SOURCE_EXTENDED 1
#include <ncursesw/curses.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include "ui.h"

static int win_rows, win_cols;

/*
bool is_in_tmux(void) {
    return getenv("TMUX") != NULL;
}
*/

/**
 * @brief prints "label: value" with a color highlight.
 */
static void draw_stat_line(int row, int col, const char* label, const char* value, int color_id, int total_width) {
    // Print the Label at the start position
    mvprintw(row, col, "%s: ", label);

    // calculate remaining space
    int label_padding = strlen(label) + 2; 
    int space_left = total_width - label_padding;

    // if there's enough room, print the value in BOLD with COLOR
    if (space_left > 0) {
        attron(COLOR_PAIR(color_id) | A_BOLD);
        
        // truncate the string if it's too long
        printw("%.*s", space_left, value);
        
        attroff(COLOR_PAIR(color_id) | A_BOLD);
    }
}

/**
 * draws decorative box
 */
void draw_box(int y, int x, int h, int w, const char* title) {
    attron(A_BOLD);
    
    // Corners 
    mvaddstr(y, x, U_TOP_LEFT);
    mvaddstr(y, x + w - 1, U_TOP_RIGHT);
    mvaddstr(y + h - 1, x, U_BOT_LEFT);
    mvaddstr(y + h - 1, x + w - 1, U_BOT_RIGHT);

    // Horizontal Lines 
    for (int i = 1; i < w - 1; i++) {
        mvaddstr(y, x + i, U_HORIZ);
        mvaddstr(y + h - 1, x + i, U_HORIZ);
    }

    // Vertical Lines 
    for (int i = 1; i < h - 1; i++) {
        mvaddstr(y + i, x, U_VERT);
        mvaddstr(y + i, x + w - 1, U_VERT);
    }

    // Title
    if (title) {
        // Use a space before and after the title to "break" the line nicely
        mvprintw(y, x + 2, " %s ", title);
    }
    
    attroff(A_BOLD);
}

/**
 * progress bar rendering
 */
static void draw_bar(int y, int x, int w, double pct) {
    int filled = (int)((pct / 100.0) * w);
    /* Green < 50%, Yellow < 80%, Red >= 80% */
    int pair = (pct > 80) ? 4 : (pct > 50 ? 3 : 2);

    mvaddch(y, x, '[');
    attron(COLOR_PAIR(pair));
    for (int i = 0; i < filled; i++) {
        mvaddch(y, x + 1 + i, ACS_CKBOARD);
    }
    attroff(COLOR_PAIR(pair));

    for (int i = filled; i < w; i++) {
        mvaddch(y, x + 1 + i, '.');
    }
    mvaddch(y, x + w + 1, ']');
}

void init_ui(void) {
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    cbreak();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    
    // set 500ms timeout for getch()
    timeout(500);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_CYAN,    -1); /* Labels */
        init_pair(2, COLOR_GREEN,   -1); /* Low usage */
        init_pair(3, COLOR_YELLOW,  -1); /* Medium usage */
        init_pair(4, COLOR_RED,     -1); /* High usage */
        init_pair(5, COLOR_MAGENTA, -1); /* Header */
    }
}

void end_ui(void) {
    endwin();
}

void render_ui(const char *distro, 
                const char *hostname, 
                double uptime, 
                const char *time_buf, 
                double cpu_load, 
                const mem_stats_t *m, 
                const cpu_stats_t *cpu,
                const disk_stats_t *disk) 
{
    (void)cpu; 

    getmaxyx(stdscr, win_rows, win_cols);
    erase();

    // width calculation 
    int box_w = (win_cols - 4 < 20) ? 20 : win_cols - 4;
    int half_w = (box_w / 2) - 1; 
    int cur_y = 1;

    // Interface
/* ------------------------------------------------------------------------- */
    // title bar 
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(cur_y++, 1, " skanna ");
    attroff(A_BOLD | COLOR_PAIR(5));
    cur_y++; 

/* ------------------------------------------------------------------------- */
    // 1st Row
    // Operating System and Current Time
    draw_box(cur_y, 0, 3, half_w, " INFO ");
    draw_stat_line(cur_y + 1, 2, "OS", distro, 1, half_w - 2);

    int second_x = half_w + 2;
    draw_box(cur_y, second_x, 3, half_w, " TIME ");
    draw_stat_line(cur_y + 1, second_x + 2, "Time", time_buf, 1, half_w - 2);
    cur_y += 4;
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
    // 2nd Row
    // Hostname
    draw_box(cur_y, 0, 3, half_w, " HOSTNAME ");
    draw_stat_line(cur_y + 1, 2, "Hostname", hostname, 1, half_w - 2);
    //cur_y += 4;

    // disk stats
    char disk_stat_fmt[64];
    snprintf(disk_stat_fmt, sizeof(disk_stat_fmt), 
            "Total: %.1fGB, Available: %.1fGB, Used: %1.fGB (%1.f%%)",
            disk->total, disk->available, disk->used, disk->percentage);
    draw_box(cur_y, second_x, 3, half_w, " DISK ");
    draw_stat_line(cur_y+1, second_x + 2, "Usage", disk_stat_fmt, 1, half_w - 4);
    cur_y += 4;
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
    // 3rd Row
    // CPU section
    draw_box(cur_y, 0, 5, box_w, " CPU ");
    int cpu_pair = (cpu_load > 80) ? 4 : (cpu_load > 50 ? 3 : 2);
    char cpu_str[16];
    snprintf(cpu_str, sizeof(cpu_str), "%.2f%%", cpu_load);
    draw_stat_line(cur_y + 1, 2, "Load", cpu_str, cpu_pair, box_w - 2);
    draw_bar(cur_y + 2, 2, box_w - 6, cpu_load);
    cur_y += 6;

    // memory section
    draw_box(cur_y, 0, 5, box_w, " MEMORY ");
    double mem_used = (double)(m->total_mem - m->avail_mem);
    double mem_pct = (m->total_mem > 0) ? (100.0 * mem_used / m->total_mem) : 0.0;


    char mem_str[64];
    // conversion from KB to MB
    snprintf(mem_str, sizeof(mem_str), "%.1f/%.1f GB (%.1f%%)", 
             mem_used / 1024.0 / 1024.0, (double)m->total_mem / 1024.0 / 1024.0, mem_pct);
    
    draw_stat_line(cur_y + 1, 2, "Usage", mem_str, 1, box_w - 2);
    draw_bar(cur_y + 2, 2, box_w - 6, mem_pct);
    cur_y += 6;

/* ------------------------------------------------------------------------- */

    // uptime (H:M:S format)
    attron(A_DIM);
    mvprintw(cur_y, 2, "Uptime: %02d:%02d:%02d", 
             (int)(uptime / 3600), ((int)uptime % 3600) / 60, (int)uptime % 60);
    attroff(A_DIM);

/* ------------------------------------------------------------------------- */

    // footer 
    attron(COLOR_PAIR(1) | A_REVERSE);
    mvprintw(win_rows - 1, 0, "  Q: Exit  |  WINDOW: %dx%d  ", win_cols, win_rows);
    // fill the rest of the line with reversed spaces
    for(int i = getcurx(stdscr); i < win_cols; i++) {
        addch(' '); 
    }
    attroff(A_REVERSE);

    refresh();
}
