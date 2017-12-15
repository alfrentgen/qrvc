#include "OutputQueue.h"

using namespace std;

OutputQueue::OutputQueue(ostream* os, int32_t capacity, int32_t chunkSize):
    m_capacity(capacity), m_outStream(os), m_queue(capacity), m_syncMtx(),
    m_flushed(true), m_nextID(0), m_chunksLoaded(0)
{
    m_queue = vector<Chunk>(m_capacity, Chunk(chunkSize, 255));
}

OutputQueue::~OutputQueue()
{
    if(m_outStream != NULL && m_outStream != &cout){
        m_outStream->flush();
        //((ifstream*)m_outStream)->close();
    }
}

int32_t OutputQueue::Put(Chunk& chunk){
    if(m_chunksLoaded == m_capacity){
        return FAIL;
    }
    swap(m_queue[m_chunksLoaded].m_inBuffer, chunk.m_inBuffer);
    swap(m_queue[m_chunksLoaded].m_outBuffer, chunk.m_outBuffer);
    m_queue[m_chunksLoaded].m_frameID = chunk.m_frameID;
    m_queue[m_chunksLoaded].m_chunkID = chunk.m_chunkID;
    m_queue[m_chunksLoaded].m_hashsum = chunk.m_hashsum;
    m_queue[m_chunksLoaded].m_rendered = chunk.m_rendered;
    m_chunksLoaded++;
    return OK;
}

int32_t OutputQueue::EstimateFlushingBufferSize(){
    int32_t size = 0;
    for(int32_t i = 0; i < m_chunksLoaded; i++){
        size += m_queue[i].m_outBuffer.size();
    }
    //size -= (size > m_chunksLoaded * 12) ? m_chunksLoaded * 12 : 0;
    return size;
}

//flush output queue if it has at leastNChunks
int32_t OutputQueue::PrepareFlush(bool simple){
    LOG("%s", "In OutputQueue::PrepareFlush()\n");
    function<bool(Chunk, Chunk)> compareChunks = [] (Chunk ch1, Chunk ch2) -> bool{
        return ch1.m_frameID < ch2.m_frameID;
    };
    sort(m_queue.begin(), m_queue.begin() + m_chunksLoaded, compareChunks);

    if(simple){
        m_flushSize = 0;
        m_flushBuffer.clear();
        for(int i = 0; i < m_chunksLoaded; i++){
            Chunk& chunk = m_queue[i];
            m_flushSize += chunk.m_outBuffer.size();
            m_flushBuffer.insert(m_flushBuffer.end(), chunk.m_outBuffer.begin(), chunk.m_outBuffer.end());
        }
        m_chunksLoaded = 0;
        return OK;
    }

    int32_t newSize = EstimateFlushingBufferSize();
    newSize = (m_flushBuffer.size() < newSize) ? newSize : m_flushBuffer.size();
    m_flushBuffer.resize(newSize);
    vector<uint8_t>::iterator flushIterator= m_flushBuffer.begin();
    char* inPtr = NULL;

    long long delta = 0;
    for(int32_t i = 0; i < m_chunksLoaded; i++){

        Chunk& chunk = m_queue[i];
        int32_t dataSize = chunk.m_outBuffer.size();
        dataSize = ((dataSize - 12) > 0) ? (dataSize - 12) : 0;

        if(!chunk.m_rendered){
            LOG("LOG: frame #%lu is not rendered!\n", chunk.m_frameID);
            continue;
        }

        chrono::time_point<chrono::steady_clock> tp1 = chrono::steady_clock::now();
        uint32_t hashsum = chunk.CalcHashsum(chunk.m_outBuffer.data(), chunk.m_outBuffer.size() - 4);
        chrono::time_point<chrono::steady_clock> tp2 = chrono::steady_clock::now();
        delta += chrono::duration_cast<chrono::microseconds>(tp2 - tp1).count();

        if(chunk.m_chunkID == m_nextID){
            if(chunk.m_hashsum == hashsum){
                inPtr = (char*)chunk.m_outBuffer.data() + 8;
                copy_n(inPtr, dataSize, flushIterator);
                flushIterator += dataSize;

                m_nextID++;
            }else{
                LOG("%s", "DecOutputQueue: Chunk ID matches, but chunk hashsum is incorrect! Skipping.\n");
            }
        }else if(chunk.m_chunkID > m_nextID){//a gap or just a bad chunk?
            //It's a correctly decoded chunk, so there is a gap.
            LOG("%s", "DecOutputQueue: Chunk ID is greater than required!\n");
            if(chunk.m_hashsum == hashsum){//
                int32_t gapSize = chunk.m_chunkID - m_nextID;
                LOG("DecOutputQueue: A gap of %d chunks acquired. Starts at %lu ends at %lu.\n", gapSize, m_nextID, chunk.m_chunkID);
                inPtr = (char*)chunk.m_outBuffer.data() + 8;
                copy_n(inPtr, dataSize, flushIterator);
                flushIterator += dataSize;
                m_nextID = ++chunk.m_chunkID;
            }
            else{//the chunk is bad, it has a lousy checksum. Just go on.
                LOG("%s", "DecOutputQueue: chunk checksum is incorrect!\n");
            }
        }
    }
    LOG("Hashsum calculation time in out queue: %d\n", delta);

    m_flushSize = flushIterator - m_flushBuffer.begin();
    m_chunksLoaded = 0;
    return OK;
}

int32_t OutputQueue::Flush(){
    LOG("%s%d\n", "Flush size = ", m_flushSize);
    //LOG("%s%d\n", "m_flushBuffer.size() = ", m_flushBuffer.size());
    m_outStream->write(m_flushBuffer.data(), m_flushSize);
    m_outStream->flush();
    m_flushSize = 0;
}

void OutputQueue::SetCapacity(uint32_t newCap){
    m_capacity = newCap;
    m_queue.resize(newCap);
}

bool OutputQueue::IsFull(){
    return m_chunksLoaded == m_capacity;
}
