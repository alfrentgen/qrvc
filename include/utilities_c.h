#ifndef UTILITIES_C_H
#define UTILITIES_C_H

#include <sys/time.h>
#include <stdio.h>

#define Q(X) #X

#define START_TIME_MEASURING_C\
    struct timeval tim;\
    gettimeofday(&tim, NULL);\
    double t1=tim.tv_sec+(tim.tv_usec/1000000.0)

#define STOP_TIME_MEASURING_C\
    gettimeofday(&tim, NULL);\
    double t2=tim.tv_sec+(tim.tv_usec/1000000.0)

#define PRINT_MEASURED_TIME_C(MESSAGE)\
        fprintf(stderr, Q(MESSAGE));\
        fprintf(stderr, ", %.6lf seconds elapsed\n", t2-t1)

#define MEASURE_OPERATION_TIME_C(MESSAGE, OPERATION)\
        START_TIME_MEASURING_C;\
        OPERATION;\
        STOP_TIME_MEASURING_C;\
        PRINT_MEASURED_TIME_C(MESSAGE)

#define MEASURE_OPTIME_C(OPERATION)\
        MEASURE_OPERATION_TIME_C(Q(OPERATION time:), OPERATION)

#define MEASURE_OPTIME_C_2(OPERATION, MESSAGE)\
        MEASURE_OPERATION_TIME_C(Q(MESSAGE time elapsed:), OPERATION)

#endif
