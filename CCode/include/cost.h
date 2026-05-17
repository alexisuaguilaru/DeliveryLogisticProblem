#ifndef COST_H
#define COST_H

#include "types.h"

double calculate_cluster_cost(
    Cluster *cluster, 
    double velocity_kmh
);

double objective_function(
    double *cluster_costs, 
    size_t num_clusters,
    double max_load,
    size_t num_points,
    bool penalization
);

#endif