#ifndef UTILITIES_H
#define UTILITIES_H

#include <chrono>
#include <ctime>

#define Q(X) #X

#define START_TIME_MEASURING\
        auto start = chrono::steady_clock::now()

#define STOP_TIME_MEASURING(UNITS)\
        auto end = chrono::steady_clock::now();\
        auto delta = chrono::duration_cast<chrono::UNITS>(end - start).count()

#define PRINT_MEASURED_TIME(MESSAGE, UNITS)\
        cerr << MESSAGE << " " << delta << " " << Q(UNITS) << "\n\n"

#define MEASURE_OPERATION_TIME(MESSAGE, OPERATION, UNITS)\
        START_TIME_MEASURING;\
        OPERATION;\
        STOP_TIME_MEASURING(UNITS);\
        PRINT_MEASURED_TIME(MESSAGE, UNITS)

#define MEASURE_OPTIME(UNITS, OPERATION)\
        MEASURE_OPERATION_TIME(Q(OPERATION time:), OPERATION, UNITS)

static std::chrono::microseconds timeCounter;

#define LOG(format, ...)    fprintf(stderr, format, __VA_ARGS__)

#endif
