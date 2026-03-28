#include <ncursesw/curses.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <locale.h>
#include "metrics.h"
#include "ui.h"

int main(void) {
    setlocale(LC_ALL, "");

    cpu_stats_t prev_cpu, curr_cpu;
    mem_stats_t mem;
    disk_stats_t disk;
    char time_buffer[64];
    
    char *distro = get_distro_name();
    char *hostname = get_hostname();

    double uptime = get_uptime();
    
    init_ui();

    get_cpu_stats(&prev_cpu);
    get_mem_stats(&mem);
    get_disk_stats("/", &disk);

    render_ui(distro, hostname, uptime, "Loading...", 0.0, &mem, &prev_cpu, &disk);

    bool running = true;
    while (running) {
        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            running = false;
            continue;
        }

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", t);

        get_cpu_stats(&curr_cpu);
        get_mem_stats(&mem);
        uptime = get_uptime();
        double cpu_load = calc_cpu_usage(&prev_cpu, &curr_cpu);

        render_ui(distro, hostname, uptime, time_buffer, cpu_load, &mem, &curr_cpu, &disk);

        prev_cpu = curr_cpu;
    }

    end_ui();
    return 0;
}
