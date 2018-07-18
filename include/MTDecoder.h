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
#include "Config.h"
#include "steganography.h"

//using namespace std;
class MTDecoder
{
public:
    MTDecoder();
    virtual ~MTDecoder();

    int32_t Init(Config& config);
    int32_t Start(bool join);
    int32_t Stop();
    int32_t ValidateConfig(Config& config);

private:
    InputQueue* m_inQ;
    OutputQueue* m_outQ;

    vector<uint8_t> m_keyFrame;
    vector<Decode*> m_jobs;
    vector<thread> m_threads;
    ifstream* m_pKeyFileStream;
    Config m_config;
    StegModule m_stegModule;
};

#endif // MTDECODER_H
