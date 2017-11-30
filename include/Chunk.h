#ifndef CHUNK_H
#define CHUNK_H
#include <pch.h>

using namespace std;

struct decodeResults {
    int32_t decodedBytes;
};

class Chunk{
public:
    Chunk(int32_t inBuffSize = 0, uint8_t val = 255);
    //Chunk(uint8_t* pInBuff, int32_t inBuffSize, uint64_t frameID);
    virtual ~Chunk();

    void Init(uint8_t val = 255);

    void SetInBuffer(uint8_t* pBuffer, int32_t size);
    vector<uint8_t>& GetInBuffer();
    void FreeInBuffer();

    void SetOutBuffer(uint8_t* pBuffer, int32_t size);
    vector<uint8_t>& GetOutBuffer();
    void FreeOutBuffer();

    uint32_t CalcHashsum(uint8_t* pBuffer, uint32_t bufSize);

public:

    uint64_t m_chunkID;
    uint64_t m_frameID;
    uint32_t m_hashsum;
    bool m_rendered;
    decodeResults m_decResults;

    vector<uint8_t> m_inBuffer;
    vector<uint8_t> m_outBuffer;
};

#endif
