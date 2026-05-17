#ifndef DISTANCE_H
#define DISTANCE_H

#include "types.h"
#include "utils.h"

double haversine_distance(
    Point a, 
    Point b
);

double calculate_transfer_time(
    Point start, 
    Point end, 
    double velocity_kmh
);

#endif