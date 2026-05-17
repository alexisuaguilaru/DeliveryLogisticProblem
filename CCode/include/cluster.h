#ifndef CLUSTER_H
#define CLUSTER_H

#include "types.h"
#include <stdbool.h>

Cluster *cluster_create(size_t initial_capacity);

void cluster_add_point(
    Cluster *cluster, 
    Point point, 
    double load
);

void cluster_free(Cluster *cluster);

int cluster_assign_closest_centroid(
    Point point, 
    CentroidSet *centroids
);

int *assign_centroids_to_points(
    CentroidSet *centroids, 
    PointWithLoad *points, 
    size_t num_points
);

Cluster *build_clusters_from_assignments(
    int *assignments, 
    size_t num_assignments,
    PointWithLoad *points,
    size_t num_points,
    size_t num_clusters
);

void free_clusters(
    Cluster *clusters, 
    size_t num_clusters
);

#endif