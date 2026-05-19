#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <string.h>
#include <math.h>

static inline double square(double x) { return x * x; }

static inline double clamp(double val, double min, double max) {
    return fmin(fmax(val, min), max);
}

char *trim_whitespace(char *str);

int parse_csv_line_double(
    char *line, 
    double *values, 
    size_t max_values
);

void compute_local_range(size_t total_elements, int rank, int size, 
                         size_t *start_out, size_t *count_out);

#endif