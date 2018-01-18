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

#define MAX_BIN_CHUNKSIZE 2953
#define NOT_MQR 0

#define COUNTER_SIZE 8
#define HASHSUM_SIZE 4



//using namespace std;
class MTEncoder
{
public:
    MTEncoder();
    virtual ~MTEncoder();

    int32_t Init(Config& config);
    int32_t Start(bool join);
    int32_t Stop();

private:
    int32_t Init(istream* is, ostream* os,
                int32_t frameWidth, int32_t frameHeight, int32_t frameRepeat, int32_t tailSize, bool invert,
                QRecLevel eccLevel, int32_t qrScale, int32_t alignment,
                uint32_t framesPerThread = 0, uint32_t nThreads = 0);

private:
    int32_t m_nThreads;
    InputQueue* m_inQ;
    OutputQueue* m_outQ;
    int32_t m_qrVersion;
    bool m_invertColors;
    int32_t m_repeats;

    vector<Encode*> m_jobs;
    vector<thread> m_threads;
};

#endif // MTENCODER_H
