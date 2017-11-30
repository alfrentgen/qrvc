#ifndef JOB_H
#define JOB_H
#include <pch.h>

class Job
{
    public:
        Job();
        virtual ~Job();
        virtual int32_t Do() = 0;
        virtual void Stop() = 0;
};

#endif // JOB_H
