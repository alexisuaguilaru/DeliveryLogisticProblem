#include "utils.h"
#include <ctype.h>
#include <stdio.h>

char *trim_whitespace(char *str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == '\0') return str;
    
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}

int parse_csv_line_double(char *line, double *values, size_t max_values) {
    int count = 0;
    char *token = strtok(line, ",");
    
    while (token && count < (int)max_values) {
        char *endptr;
        values[count] = strtod(trim_whitespace(token), &endptr);
        if (endptr == token) continue;
        count++;
        token = strtok(NULL, ",");
    }
    return count;
}

void compute_local_range(size_t total, int rank, int size,
                                size_t *local_start, size_t *local_count) {
    size_t base = total / size;
    size_t rem = total % size;
    *local_count = base + (rank < (int)rem ? 1 : 0);
    *local_start = rank * base + (rank < (int)rem ? rank : (int)rem);
}