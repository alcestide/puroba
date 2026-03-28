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

/**
 * @brief Fully renders the program user interface.
 * @param distro   [in] String containing the OS distribution name.
 * @param uptime   [in] Total system uptime in seconds.
 * @param time_buf [in] Formatted string of the current system time.
 * @param cpu_load [in] Current CPU load as a percentage (0.0 - 100.0).
 * @param m        [in] Pointer to the memory statistics structure.
 * @param cpu      [in] Pointer to the raw CPU ticks structure.
 */
void render_ui(const char *distro, 
               const char *hostname,
               double uptime, 
               const char *time_buf, 
               double cpu_load, 
               const mem_stats_t *m, 
               const cpu_stats_t *cpu,
            const disk_stats_t *disk); 

#endif
