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
        try{
            ((ofstream*)m_outStream)->close();
        }catch(...){
        }
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
    m_queue[m_chunksLoaded].m_inHash = chunk.m_inHash;
    m_queue[m_chunksLoaded].m_outHash = chunk.m_outHash;
    m_queue[m_chunksLoaded].m_rendered = chunk.m_rendered;
    m_chunksLoaded++;
    return OK;
}

int32_t OutputQueue::EstimateFlushingBufferSize(){
    int32_t size = 0;
    for(int32_t i = 0; i < m_chunksLoaded; i++){
        size += m_queue[i].m_outBuffer.size();
    }
    return size;
}

//flush output queue if it has at leastNChunks
int32_t OutputQueue::PrepareFlush(bool simple){

    vector<Chunk*> snapshot(0);
    int32_t nChunks = GetSnapshot(snapshot);
    function<bool(Chunk*, Chunk*)> comparePChunks = [] (Chunk* ch1, Chunk* ch2) -> bool{
        return ch1->m_frameID < ch2->m_frameID;
    };

    stable_sort(snapshot.begin(), snapshot.begin() + nChunks, comparePChunks);

    if(simple){
        m_flushSize = 0;
        m_flushBuffer.clear();
        for(int i = 0; i < m_chunksLoaded; i++){
            Chunk& chunk = *snapshot[i];
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
        Chunk& chunk = *snapshot[i];
        int32_t dataSize = chunk.m_outBuffer.size();
        dataSize = ((dataSize - 12) > 0) ? (dataSize - 12) : 0;

        if(!chunk.m_rendered || chunk.m_chunkID < m_nextID){
            continue;
        }

        inPtr = (char*)chunk.m_outBuffer.data() + 8;
        copy_n(inPtr, dataSize, flushIterator);
        flushIterator += dataSize;

        if(chunk.m_chunkID == m_nextID){
            m_nextID++;
        }else if(chunk.m_chunkID > m_nextID){//a gap
            LOG("%s", "DecOutputQueue: Chunk ID is greater than required!\n");
            int32_t gapSize = chunk.m_chunkID - m_nextID;
            LOG("DecOutputQueue: A gap of %d chunks acquired. Starts at %lu ends at %lu.\n", gapSize, m_nextID, chunk.m_chunkID);
            m_nextID = ++chunk.m_chunkID;
        }
    }

    m_flushSize = flushIterator - m_flushBuffer.begin();
    m_flushBuffer.resize(m_flushSize);
    m_chunksLoaded = 0;
    return OK;
}

int32_t OutputQueue::Flush(){
    m_outStream->write(m_flushBuffer.data(), m_flushBuffer.size());
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

bool OutputQueue::IsAlreadyPut(Chunk& chunk){
    for(int i = 0 ; i < m_chunksLoaded; i++){
        if(m_queue[i].m_inHash == chunk.m_inHash){
            if(m_queue[i].m_inBuffer == chunk.m_inBuffer){
                return true;
            }
        }
    }
    return false;
}

int32_t OutputQueue::GetSnapshot(vector<Chunk*>& snapshot){
    snapshot.clear();
    for(int i = 0; i < m_chunksLoaded; i++){
        snapshot.push_back(&m_queue[i]);
    }
    return m_chunksLoaded;
}
