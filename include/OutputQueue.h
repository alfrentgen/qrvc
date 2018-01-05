#ifndef OUTPUTQUEUE_H
#define OUTPUTQUEUE_H

#include "pch.h"
#include "Chunk.h"

#define DROPPED 1
#define NOT_ENOUGH_CHUNKS -1
#define SIMPLE_FLUSH true

using namespace std;

class OutputQueue
{
    public:
        OutputQueue(ostream* os, int32_t capacity = 2, int32_t chunkSize = 512);
        virtual ~OutputQueue();
        int32_t Put(Chunk& ch);
        int32_t PrepareFlush(bool simple);
        int32_t Flush();
        void SetCapacity(uint32_t newCap);
        bool IsFull();
        int32_t EstimateFlushingBufferSize();
        bool IsAlreadyPut(Chunk& chunk);

    public:
        mutex m_syncMtx;
        mutex m_flushMtx;
        bool m_flushed;
        condition_variable m_cv;

    private:
        uint64_t m_nextID;
        int32_t m_capacity;
        int32_t m_chunksLoaded;
        int32_t m_chunkSize;
        int32_t m_flushSize;

        vector<Chunk> m_queue;
        vector<uint8_t> m_flushBuffer;
        ostream* m_outStream;

};

#endif // OUTPUTQUEUE_H
