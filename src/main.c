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
    net_stats_t network;
    char time_buffer[64];
    
    char *distro = get_distro_name();
    char *hostname = get_hostname();
    double uptime = get_uptime();
    
    int active_tab = 0;

    init_ui();

    get_cpu_stats(&prev_cpu);
    get_mem_stats(&mem);
    get_disk_stats("/", &disk);
    get_net_stats(&network); 

    bool running = true;
    while (running) {
        int ch = getch();
        
        // Handle input
        if (ch == 'q' || ch == 'Q') {
            running = false;
            continue;
        }
        if (ch == '1') {
            active_tab = 0;
        }
        if (ch == '2') {
            active_tab = 1;
        }

        // Update Time
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", t);

        // Update Metrics
        get_cpu_stats(&curr_cpu);
        get_mem_stats(&mem);
        get_disk_stats("/", &disk);
        get_net_stats(&network); // Update network bytes/rates
        
        uptime = get_uptime();
        double cpu_load = calc_cpu_usage(&prev_cpu, &curr_cpu);

        // Render with new tab parameter
        render_ui(distro, hostname, uptime, time_buffer, cpu_load, 
                  &mem, &curr_cpu, &disk, &network, active_tab);

        prev_cpu = curr_cpu;
        
        // The 500ms timeout is handled by timeout(500) in init_ui()
    }

    end_ui();
    return 0;
}
