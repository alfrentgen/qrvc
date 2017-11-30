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
enum DecodeLib{
    QUIRC = 0,
    ZBAR = 1
};


class MTDecoder
{
    public:
        MTDecoder();
        virtual ~MTDecoder();

        uint32_t Init(istream* is, ostream* os, int32_t frameWidth, int32_t frameHeight, DecodeLib decLib = ZBAR, uint32_t framesPerThread = 0, uint32_t nThreads = 0);
        uint32_t Start(bool join);
        uint32_t Stop();

    private:
        int32_t m_nThreads;
        InputQueue* m_inQ;
        OutputQueue* m_outQ;
        DecodeLib m_decLib;

        vector<Decode*> m_jobs;
        vector<thread> m_threads;
};

#endif // MTDECODER_H
