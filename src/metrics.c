#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/statvfs.h>
#include "metrics.h"

static unsigned long long total_ticks(const cpu_stats_t *s) {
    return s->user + s->nice + s->system + s->idle + s->iowait + s->irq + s->softirq;
}

void get_cpu_stats(cpu_stats_t *s) {
    FILE *fp = fopen("/proc/stat", "r");
    if (fp) {
        fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu",
               &s->user, &s->nice, &s->system, &s->idle,
               &s->iowait, &s->irq, &s->softirq);
        fclose(fp);
    }
}

void get_mem_stats(mem_stats_t *s) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) return;

    char label[32];
    unsigned long val;
    int count = 0;
    while (fscanf(fp, "%31s %lu %*s", label, &val) != EOF && count < 2) {
        if (strcmp(label, "MemTotal:") == 0) {
            s->total_mem = val;
            count++;
        } else if (strcmp(label, "MemAvailable:") == 0) {
            s->avail_mem = val;
            count++;
        }
    }
    fclose(fp);
}

double calc_cpu_usage(const cpu_stats_t *prev, const cpu_stats_t *curr) {
    unsigned long long total_d = total_ticks(curr) - total_ticks(prev);
    unsigned long long idle_d = (curr->idle + curr->iowait) - (prev->idle + prev->iowait);
    return (total_d == 0) ? 0.0 : (double)(total_d - idle_d) / total_d * 100.0;
}

char *get_hostname(){
    static char hostname[256] = "unknown";
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        //printf("Hostname: %s\n", hostname);
        return hostname;
        return 0;
    } else {
        perror("gethostname");
    }
    return hostname;
}

char *get_distro_name(void) {
    static char distro[128] = "Unix (Generic)";
    FILE *fp = fopen("/etc/os-release", "r");
    if (!fp) return distro;

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
            char *start = strchr(line, '"');
            char *end = strrchr(line, '"');
            if (start && end && start != end) {
                *end = '\0';
                strncpy(distro, start + 1, sizeof(distro)-1);
            }
            break;
        }
    }
    fclose(fp);
    return distro;
}

double get_uptime(void) {
    double uptime = 0.0;
    FILE *fp = fopen("/proc/uptime", "r");
    if (fp) {
        fscanf(fp, "%lf", &uptime);
        fclose(fp);
    }
    return uptime;
}

void get_disk_stats(const char *path, disk_stats_t *stats) {
    const unsigned long long GB = 1024ULL * 1024 * 1024;
    struct statvfs buffer;

    if (statvfs(path, &buffer) == 0) {
        stats->total = ((double)buffer.f_blocks * buffer.f_frsize) / GB;
        stats->available = ((double)buffer.f_bavail * buffer.f_frsize) / GB;
        stats->used = stats->total - stats->available;
        stats->percentage = (stats->used / stats->total) * 100.0;
    } else {
        stats->success = 0;
    }
}
