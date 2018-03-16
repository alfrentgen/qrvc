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
#include "Config.h"

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
    int32_t ValidateConfig(Config& config);

private:
    InputQueue* m_inQ;
    OutputQueue* m_outQ;
    vector<uint8_t> m_keyQR;
    ofstream* m_pKeyFileStream;
    Config m_config;

    vector<Encode*> m_jobs;
    vector<thread> m_threads;
};

#endif // MTENCODER_H
