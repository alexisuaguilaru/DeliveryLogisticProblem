#include "cluster.h"
#include "distance.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_CLUSTER_CAPACITY 50

Cluster *cluster_create(size_t initial_capacity) {
    Cluster *c = malloc(sizeof(Cluster));
    if (!c) return NULL;
    
    c->points = malloc(initial_capacity * sizeof(Point));
    if (!c->points) {
        free(c);
        return NULL;
    }
    
    c->count = 0;
    c->capacity = initial_capacity;
    c->total_load = 0.0;
    return c;
}

void cluster_add_point(Cluster *cluster, Point point, double load) {
    if (cluster->count >= cluster->capacity) {
        size_t new_cap = cluster->capacity * 2;
        Point *new_points = realloc(cluster->points, new_cap * sizeof(Point));
        if (!new_points) return;
        cluster->points = new_points;
        cluster->capacity = new_cap;
    }
    cluster->points[cluster->count++] = point;
    cluster->total_load += load;
}

void cluster_free(Cluster *cluster) {
    if (cluster) {
        free(cluster->points);
        free(cluster);
    }
}

int cluster_assign_closest_centroid(Point point, CentroidSet *centroids) {
    int closest = 0;
    double min_dist = haversine_distance(point, centroids->centroids[0]);
    
    for (size_t i = 1; i < centroids->num_centroids; ++i) {
        double d = haversine_distance(point, centroids->centroids[i]);
        if (d < min_dist) {
            min_dist = d;
            closest = (int)i;
        }
    }
    return closest;
}

int *assign_centroids_to_points(CentroidSet *centroids,
                                PointWithLoad *points,
                                size_t num_points) {
    int *assignments = malloc(num_points * sizeof(int));
    if (!assignments) return NULL;
    
    for (size_t i = 0; i < num_points; ++i) {
        assignments[i] = cluster_assign_closest_centroid(points[i].point, centroids);
    }
    return assignments;
}

static int compare_by_longitude(const void *a, const void *b) {
    const Point *pa = (const Point *)a;
    const Point *pb = (const Point *)b;
    return (pa->lon > pb->lon) - (pa->lon < pb->lon);
}

Cluster *build_clusters_from_assignments(int *assignments,
                                         size_t num_assignments,
                                         PointWithLoad *points,
                                         size_t num_points,
                                         size_t num_clusters) {
    if (num_assignments != num_points) return NULL;
    
    Cluster *clusters = calloc(num_clusters, sizeof(Cluster));
    if (!clusters) return NULL;
    
    for (size_t i = 0; i < num_clusters; ++i) {
        clusters[i] = *cluster_create(INITIAL_CLUSTER_CAPACITY);
    }
    
    for (size_t i = 0; i < num_points; ++i) {
        int cid = assignments[i];
        if (cid >= 0 && cid < (int)num_clusters) {
            cluster_add_point(&clusters[cid], points[i].point, points[i].load);
        }
    }
    
    for (size_t i = 0; i < num_clusters; ++i) {
        if (clusters[i].count > 1) {
            qsort(clusters[i].points, clusters[i].count, sizeof(Point), compare_by_longitude);
        }
    }
    
    return clusters;
}

void free_clusters(Cluster *clusters, size_t num_clusters) {
    if (clusters) {
        for (size_t i = 0; i < num_clusters; ++i) {
            free(clusters[i].points);
        }
        free(clusters);
    }
}