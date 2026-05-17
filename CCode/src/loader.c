#include "config.h"
#include "loader.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t count_csv_lines(const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) return 0;
    
    size_t count = 0;
    char buffer[LINE_BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), f)) {
        if (trim_whitespace(buffer)[0] != '\0') count++;
    }

    fclose(f);
    return count;
}

PointWithLoad *load_points_csv(const char *filepath, size_t *out_count) {
    FILE *f = fopen(filepath, "r");
    if (!f) return NULL;
    
    size_t capacity = 64;
    size_t count = 0;
    PointWithLoad *points = malloc(capacity * sizeof(PointWithLoad));
    if (!points) { fclose(f); return NULL; }
    
    char line[LINE_BUFFER_SIZE];
    while (fgets(line, sizeof(line), f)) {
        char *trimmed = trim_whitespace(line);
        if (trimmed[0] == '\0') continue;
        
        double values[3];
        if (parse_csv_line_double(trimmed, values, 3) != 3) continue;
        
        if (count >= capacity) {
            capacity *= 2;
            PointWithLoad *tmp = realloc(points, capacity * sizeof(PointWithLoad));
            if (!tmp) { free(points); fclose(f); return NULL; }
            points = tmp;
        }
        
        points[count].point.lon = values[0];
        points[count].point.lat = values[1];
        points[count].load = values[2];
        count++;
    }
    
    fclose(f);
    *out_count = count;
    return points;
}

CentroidSet *load_centroids_csv(const char *filepath, size_t num_centroids) {
    FILE *f = fopen(filepath, "r");
    if (!f) return NULL;
    
    CentroidSet *set = malloc(sizeof(CentroidSet));
    if (!set) { fclose(f); return NULL; }
    
    set->centroids = malloc(num_centroids * sizeof(Point));
    if (!set->centroids) { free(set); fclose(f); return NULL; }
    
    char content[4096];
    size_t read = fread(content, 1, sizeof(content) - 1, f);
    content[read] = '\0';
    fclose(f);
    
    char *token = strtok(content, ",");
    size_t idx = 0;
    
    while (token && idx < num_centroids) {
        set->centroids[idx].lon = atof(token);
        token = strtok(NULL, ",");
        if (!token) break;
        set->centroids[idx].lat = atof(token);
        token = strtok(NULL, ",");
        idx++;
    }
    
    set->num_centroids = num_centroids;
    return set;
}

void free_points(PointWithLoad *points, size_t count) {
    (void)count;
    free(points);
}

void free_centroids(CentroidSet *centroids) {
    if (centroids) {
        free(centroids->centroids);
        free(centroids);
    }
}

int *get_dataset_range_partitions(const char *filepath, 
                                  int num_workers, 
                                  size_t *out_partition_sizes) {
    size_t total_lines = count_csv_lines(filepath);
    if (total_lines == 0) return NULL;
    
    int *partitions = malloc((num_workers + 1) * sizeof(int));
    if (!partitions) return NULL;
    
    size_t base = total_lines / num_workers;
    size_t remainder = total_lines % num_workers;
    
    partitions[0] = 0;
    for (int i = 0; i < num_workers; ++i) {
        size_t extra = (i < (int)remainder) ? 1 : 0;
        partitions[i + 1] = partitions[i] + (int)(base + extra);
    }
    
    if (out_partition_sizes) {
        for (int i = 0; i < num_workers; ++i) {
            out_partition_sizes[i] = partitions[i + 1] - partitions[i];
        }
    }
    
    return partitions;
}