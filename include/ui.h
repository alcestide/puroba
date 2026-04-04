#ifndef UI_H
#define UI_H

#include "metrics.h"

#define U_TOP_LEFT     "┌"
#define U_TOP_RIGHT    "┐"
#define U_BOT_LEFT     "└"
#define U_BOT_RIGHT    "┘"
#define U_HORIZ        "─"
#define U_VERT         "│"
#define U_DIVIDER_L    "├"
#define U_DIVIDER_R    "┤"

/** @brief Initializes ncurses, sets up colors, and 500ms input timeout.
 * @note Must be called before any other UI function. */
void init_ui(void);

/** @brief Restores the terminal to its original state and closes ncurses. */
void end_ui(void);

/** * @brief Draws a decorative box with an optional title.
 */
void draw_box(int y, int x, int h, int w, const char* title);

/**
 * @brief Fully renders the program user interface.
 * @param distro        [in] String containing the OS distribution name.
 * @param hostname      [in] String containing the system hostname.
 * @param uptime        [in] Total system uptime in seconds.
 * @param time_buf      [in] Formatted string of the current system time.
 * @param cpu_load      [in] Current CPU load as a percentage (0.0 - 100.0).
 * @param m             [in] Pointer to the memory statistics structure.
 * @param cpu           [in] Pointer to the raw CPU ticks structure.
 * @param disk          [in] Pointer to the disk statistics structure.
 * @param net           [in] Pointer to the net statistics structure.
 * @param active_tab    [in] Current active UI tab.
 */
void render_ui(const char *distro, 
               const char *hostname,
               double uptime, 
               const char *time_buf, 
               double cpu_load, 
               const mem_stats_t *m, 
               const cpu_stats_t *cpu,
               const disk_stats_t *disk,
               const net_stats_t *net,
               int active_tab); 

#endif
