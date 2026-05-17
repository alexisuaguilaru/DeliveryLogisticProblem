#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    double lon;
    double lat;
} Point;

typedef struct {
    Point point;
    double load;
} PointWithLoad;

typedef struct {
    Point point;
    double load;
    int original_index; 
} ClusterPoint;

typedef struct {
    ClusterPoint *points;
    size_t count;
    size_t capacity;
    double total_load;
} Cluster;

typedef struct {
    Point *centroids;
    size_t num_centroids;
} CentroidSet;

typedef struct {
    char *dataset_path;
    char *solution_path;
    char *results_name;
    int num_workers;
    int num_clusters;
    double max_load;
    bool penalization;
} ProgramArgs;

typedef struct {
    double fitness;
    double time_total;
    double time_partition;
    double time_assign;
    double time_cluster;
    double time_cost;
} ExecutionMetrics;

#endif