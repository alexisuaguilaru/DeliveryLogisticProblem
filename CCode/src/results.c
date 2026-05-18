#include "results.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>
#include <string.h>

static int ensure_results_dir(void) {
    return mkdir("./results", 0755) == 0 || errno == EEXIST;
}

static int mkdirs(const char *path) {
    char tmp[512];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }
    
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}


static void get_parent_dir(const char *filepath, char *dirbuf, size_t bufsize) {
    strncpy(dirbuf, filepath, bufsize - 1);
    dirbuf[bufsize - 1] = '\0';
    
    char *last_slash = strrchr(dirbuf, '/');
    if (last_slash) {
        *last_slash = '\0'; 
    } else {
        
        strncpy(dirbuf, ".", bufsize - 1);
        dirbuf[bufsize - 1] = '\0';
    }
}

int dump_clusters_results(Cluster *clusters,
                          size_t num_clusters,
                          double *costs,
                          const char *results_name) {
    if (!ensure_results_dir()) return -1;
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "./results/%s_clusters.csv", results_name);
    
    FILE *f = fopen(filepath, "w");
    if (!f) return -1;
    
    fprintf(f, "Cost,Cluster\n");
    for (size_t i = 0; i < num_clusters; ++i) {
        fprintf(f, "%f,", costs[i]);
        fprintf(f,"\"[");
        for (size_t j = 0; j < clusters[i].count; ++j) {
            if (j > 0) fputc(',', f);
            fprintf(f, "%d", clusters[i].points[j].original_index);
        }
        fprintf(f, "]\"\n");
    }
    
    fclose(f);
    return 0;
}

int dump_info_results(ExecutionMetrics *metrics, const char *results_name) {
    if (!ensure_results_dir()) return -1;
    
    char filepath[512];
    char dirpath[512];

    snprintf(filepath, sizeof(filepath), "./results/%s_info.csv", results_name);
    get_parent_dir(filepath, dirpath, sizeof(dirpath));

    if (mkdirs(dirpath) != 0) return -1;
    
    FILE *f = fopen(filepath, "w");
    if (!f) return -1;
    
    fprintf(f, "Parameter,Value\n");
    fprintf(f, "Fitness,%f\n", metrics->fitness);
    fprintf(f, "TimeExecution,%f\n", metrics->time_total);
    fprintf(f, "TimeDatasetPartition,%f\n", metrics->time_partition);
    fprintf(f, "TimeCentroidsAssignation,%f\n", metrics->time_assign);
    fprintf(f, "TimeClusters,%f\n", metrics->time_cluster);
    fprintf(f, "TimeCalculateCosts,%f\n", metrics->time_cost);
    fprintf(f, "RealTime,%f\n", metrics->real_time);
    fprintf(f, "UserTime,%f\n", metrics->user_time);
    fprintf(f, "SysTime,%f\n", metrics->sys_time);
    fprintf(f, "CPUWall,%f\n", metrics->cpu_wall);
    
    fclose(f);
    return 0;
}