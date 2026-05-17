#ifndef LOADER_H
#define LOADER_H

#include "types.h"
#include <stdio.h>

PointWithLoad *load_points_csv(
    const char *filepath, 
    size_t *out_count
);

CentroidSet *load_centroids_csv(
    const char *filepath, 
    size_t num_centroids
);

void free_points(
    PointWithLoad *points, 
    size_t count
);

void free_centroids(
    CentroidSet *centroids
);

size_t count_csv_lines(const char *filepath);

int *get_dataset_range_partitions(
    const char *filepath, 
    int num_workers, 
    size_t *out_partition_sizes
);

#endif