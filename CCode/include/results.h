#ifndef RESULTS_H
#define RESULTS_H

#include "types.h"

int dump_clusters_results(
    Cluster *clusters, 
    size_t num_clusters,
    double *costs,
    const char *results_name
);

int dump_info_results(
    ExecutionMetrics *metrics, 
    const char *results_name
);

#endif