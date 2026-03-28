#ifndef METRICS_H
#define METRICS_H

/** @struct cpu_stats_t @brief Raw CPU ticks from /proc/stat */
typedef struct {
    unsigned long long user, nice, system, idle, iowait, irq, softirq;
} cpu_stats_t;

/** @struct mem_stats_t @brief Memory statistics in KBs */
typedef struct {
    unsigned long total_mem, avail_mem;
} mem_stats_t;

/** @struct disk_stats_t @brief Storage metrics. */
typedef struct {
    double total;
    double available;
    double used;
    double percentage;
    int success;
} disk_stats_t;

/** @brief Fetches current CPU ticks into s */
void get_cpu_stats(cpu_stats_t *s);

/** @brief Fetches current memory info into s */
void get_mem_stats(mem_stats_t *s);

/** @brief Returns system uptime in seconds from /proc/uptime */
double get_uptime(void);

/** @brief Returns static string containing the OS distribution name.
 * @warning Currently only works on Linux. To-do: Unix/MacOS */
char *get_distro_name(void);

/** @brief Return static string containing the system hostname. */
char *get_hostname(void);

/** @brief Fetches disk usage for a given mount path (e.g., "/").
 * @param path [in] Mount point string | 
 * @param stats [out] Struct to fill.
 * @note Uses statfs() to calculate total and available blocks. */
void get_disk_stats(const char *path, disk_stats_t *stats);

/** @brief Calculates CPU load % (delta) between two samples
 * @return Percentage (0.0 to 100.0) or 0.0 on zero delta */
double calc_cpu_usage(const cpu_stats_t *prev, const cpu_stats_t *curr);

#endif
