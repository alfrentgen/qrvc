#ifndef MTENCODER_H
#define MTENCODER_H

#include "pch.h"
#include "inputQueue.h"
#include "OutputQueue.h"
#include "Worker.h"
#include "Job.h"
#include "Encode.h"
#include "ArgsParser.h"
#include "Chunk.h"

#define COUNTER_SIZE 8
#define HASHSUM_SIZE 4

//using namespace std;
class MTEncoder
{
    public:
        MTEncoder();
        virtual ~MTEncoder();

        uint32_t Init(istream* is, ostream* os, int32_t frameWidth, int32_t frameHeight, uint32_t chunkSize,
                       uint32_t framesPerThread = 0, uint32_t nThreads = 0);
        uint32_t Start(bool join);
        uint32_t Stop();

    private:
        int32_t m_nThreads;
        InputQueue* m_inQ;
        OutputQueue* m_outQ;

        vector<Encode*> m_jobs;
        vector<thread> m_threads;
};

#endif // MTENCODER_H
