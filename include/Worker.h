#include <pch.h>
#include <Job.h>

#ifndef WORKER_H
#define WORKER_H

using namespace std;

enum JobState{DONE, DOING};

class Worker
{
    public:
        Worker();
        virtual ~Worker();
        int32_t Start();
        void Stop();
        void SetJob(Job* j);

    private:
        void Work();

    private:
        condition_variable m_cv;//this is for wait/notify working thread
        mutex m_uniqThreadMtx;
        mutex m_syncMtx;

        bool m_isStopped;
        Job* m_job;
        thread m_thread;

        JobState m_jobState;
};

#endif // WORKER_H
