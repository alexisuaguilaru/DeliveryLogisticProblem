#include "results.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

static int ensure_results_dir(void) {
    return mkdir("./results", 0755) == 0 || errno == EEXIST;
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
        fputc('"', f);
        for (size_t j = 0; j < clusters[i].count; ++j) {
            if (j > 0) fputc(',', f);
            fprintf(f, "%zu", j);
        }
        fprintf(f, "\"\n");
    }
    
    fclose(f);
    return 0;
}

int dump_info_results(ExecutionMetrics *metrics, const char *results_name) {
    if (!ensure_results_dir()) return -1;
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "./results/%s_info.csv", results_name);
    
    FILE *f = fopen(filepath, "w");
    if (!f) return -1;
    
    fprintf(f, "Parameter,Value\n");
    fprintf(f, "Fitness,%f\n", metrics->fitness);
    fprintf(f, "TimeExecution,%f\n", metrics->time_total);
    fprintf(f, "TimeDatasetPartition,%f\n", metrics->time_partition);
    fprintf(f, "TimeCentroidsAssignation,%f\n", metrics->time_assign);
    fprintf(f, "TimeClusters,%f\n", metrics->time_cluster);
    fprintf(f, "TimeCalculateCosts,%f\n", metrics->time_cost);
    
    fclose(f);
    return 0;
}