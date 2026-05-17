#include "cost.h"
#include "distance.h"
#include "config.h"
#include <math.h>
#include <stdlib.h>

double calculate_cluster_cost(Cluster *cluster, double velocity_kmh) {
    if (!cluster || cluster->count == 0) return 0.0;
    
    double cost = 0.0;
    
    if (cluster->count > 1) {
        for (size_t i = 0; i < cluster->count - 1; ++i) {
            cost += calculate_transfer_time(
                cluster->points[i].point, 
                cluster->points[i + 1].point, 
                velocity_kmh
            );

        }

        cost += calculate_transfer_time(
            cluster->points[cluster->count - 1].point,
            cluster->points[0].point,
            velocity_kmh
        );
    }
    
    cost += cluster->total_load;
    return cost;
}

static double calculate_mean(const double *values, size_t n) {
    if (n == 0) return 0.0;
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i) sum += values[i];
    return sum / n;
}

static double calculate_stdev(const double *values, size_t n) {
    if (n < 2) return 0.0;
    double mean = calculate_mean(values, n);
    double sum_sq = 0.0;
    for (size_t i = 0; i < n; ++i) {
        sum_sq += (values[i] - mean) * (values[i] - mean);
    }
    return sqrt(sum_sq / (n - 1));
}

double objective_function(double *cluster_costs,
                          size_t num_clusters,
                          double max_load,
                          size_t num_points,
                          bool penalization) {
    double fitness = calculate_stdev(cluster_costs, num_clusters);
    
    if (penalization) {
        double penalty_sum = 0.0;
        for (size_t i = 0; i < num_clusters; ++i) {
            if (cluster_costs[i] > max_load) {
                penalty_sum += max_load;
            }
        }
        fitness += penalty_sum / num_points;
    }
    
    return fitness;
}