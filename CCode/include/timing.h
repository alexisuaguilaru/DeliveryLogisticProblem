#ifndef TIMMING_H
#define TIMMING_H

#ifdef __cplusplus
extern "C" {
#endif


#include <sys/resource.h>
#include <sys/time.h>

void uswtime(double *usertime, double *systime, double *walltime);


#ifdef __cplusplus
}
#endif

#endif