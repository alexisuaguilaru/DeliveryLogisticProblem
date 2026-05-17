#include "distance.h"
#include "config.h"
#include <math.h>

static double to_radians(double degrees) {
    return degrees * M_PI / 180.0;
}

double haversine_distance(Point a, Point b) {
    double lat1 = to_radians(a.lat);
    double lat2 = to_radians(b.lat);
    double lon1 = to_radians(a.lon);
    double lon2 = to_radians(b.lon);
    
    double cos_dlon = cos(lon1 - lon2);
    double cos_dlat = cos(lat1 - lat2);
    double cos_sum_lat = cos(lat1 + lat2);
    
    double term1 = (1.0 + cos_dlon) * cos_dlat;
    double term2 = (1.0 - cos_dlon) * cos_sum_lat;
    
    double acos_arg = 0.5 * (term1 - term2);
    acos_arg = clamp(acos_arg, -1.0, 1.0);
    
    return EARTH_RADIUS_KM * acos(acos_arg) + 1.0;
}

double calculate_transfer_time(Point start, Point end, double velocity_kmh) {
    double distance = haversine_distance(start, end);
    return 3600.0 * distance / velocity_kmh;
}