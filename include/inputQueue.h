#ifndef INPUTQUEUE_H
#define INPUTQUEUE_H
#include "pch.h"
#include "Chunk.h"

#define INQ_EMPTY_AND_DEPLETED -2
#define INQ_EMPTY -1
#define DROP_TAIL true

using namespace std;

class InputQueue
{
    public:
        InputQueue(istream* stream, int32_t capacity = 2, int32_t chunkSize = 512);
        virtual ~InputQueue();

        int32_t GetChunk(Chunk& chunkTo);
        int32_t Load(bool dropTail);

    public:
        bool m_waitForFlush;
        int32_t m_chunksAvailable;
        mutex m_syncMtx;
        condition_variable m_cv;

    private:
        istream* m_inStream;
        vector<Chunk> m_storage;
        deque<Chunk*> m_queue;

        bool m_depleted;
        int32_t m_capacity;
        int32_t m_chunkSize;
        uint64_t m_frameCounter;
};

#endif
