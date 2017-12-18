#ifndef MTDECODER_H
#define MTDECODER_H

#include "pch.h"
#include "inputQueue.h"
#include "OutputQueue.h"
#include "Worker.h"
#include "Job.h"
#include "Decode.h"
#include "ArgsParser.h"
#include "Chunk.h"

//using namespace std;
class MTDecoder
{
    public:
        MTDecoder();
        virtual ~MTDecoder();

        int32_t Init(istream* is, ostream* os, int32_t frameWidth, int32_t frameHeight, DecodeMode decMode = MIXED, uint32_t framesPerThread = 0, uint32_t nThreads = 0);
        int32_t Init(Config& config);
        int32_t Start(bool join);
        int32_t Stop();

    private:
        int32_t m_nThreads;
        InputQueue* m_inQ;
        OutputQueue* m_outQ;
        DecodeMode m_decMode;

        vector<Decode*> m_jobs;
        vector<thread> m_threads;
};

#endif // MTDECODER_H
