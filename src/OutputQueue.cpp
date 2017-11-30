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
    size -= (size > m_chunksLoaded * 12) ? m_chunksLoaded * 12 : 0;
    return size;
}

//flush output queue if it has at leastNChunks
int32_t OutputQueue::PrepareFlush(){
    cerr << "In OutputQueue::Flush()\n";
    function<bool(Chunk, Chunk)> compareChunks = [] (Chunk ch1, Chunk ch2) -> bool{
        return ch1.m_frameID < ch2.m_frameID;
    };

    sort(m_queue.begin(), m_queue.begin() + m_chunksLoaded, compareChunks);

    int32_t newSize = EstimateFlushingBufferSize();
    newSize = m_flushBuffer.size() < newSize ? newSize : m_flushBuffer.size();
    m_flushBuffer.resize(newSize);
    vector<uint8_t>::iterator flushIterator= m_flushBuffer.begin();
    char* inPtr = NULL;
    uint32_t dataSize = 0;

    for(int32_t i = 0; i < m_chunksLoaded; i++){

        Chunk& chunk = m_queue[i];
        //fprintf(stderr, "DecOutputQueue: Parsing chunk: frame_ID = %lu, chunk_ID = %lu, size = %d\n", chunk.m_frameID, chunk.m_chunkID, chunk.m_outBuffer.size());
        if(!chunk.m_rendered){
            fprintf(stderr, "LOG: frame #%lu is not rendered!", chunk.m_frameID);
            continue;
        }

        if(/*true*/chunk.m_chunkID == m_nextID){
            uint32_t hashsum = chunk.CalcHashsum(chunk.m_outBuffer.data(), chunk.m_outBuffer.size()-4);
            if(/*true*/chunk.m_hashsum == chunk.CalcHashsum(chunk.m_outBuffer.data(), chunk.m_outBuffer.size()-4)){
                //cerr << "Writing: chunk.m_frameID = " << chunk.m_frameID << ", size = " << chunk.m_outBuffer.size() << endl;

                //m_outStream->write((char*)chunk.m_outBuffer.data()+8, chunk.m_outBuffer.size()-12);
                inPtr = (char*)chunk.m_outBuffer.data() + 8;
                dataSize = chunk.m_outBuffer.size() - 12;
                copy_n(inPtr, dataSize, flushIterator);
                flushIterator += dataSize;

                m_nextID++;
            }else{
                fprintf(stderr, "DecOutputQueue: Chunk ID matches, but chunk hashsum is incorrect! Skipping.\n");
            }
        }else if(chunk.m_chunkID > m_nextID){//a gap or just a bad chunk?
            //It's a correctly decoded chunk, so there is a gap.
            fprintf(stderr, "DecOutputQueue: Chunk ID is greater than required!\n");
            uint32_t hashsum = chunk.CalcHashsum(chunk.m_outBuffer.data(), chunk.m_outBuffer.size()-4);
            if(/*true*/chunk.m_hashsum == chunk.CalcHashsum(chunk.m_outBuffer.data(), chunk.m_outBuffer.size()-4)){//
                int32_t gapSize = chunk.m_chunkID - m_nextID;
                fprintf(stderr, "DecOutputQueue: A gap of %d chunks acquired. Starts at %lu ends at %lu.\n", gapSize, m_nextID, chunk.m_chunkID);

                //m_outStream->write((char*)chunk.m_outBuffer.data()+8, chunk.m_outBuffer.size()-12);
                inPtr = (char*)chunk.m_outBuffer.data() + 8;
                dataSize = chunk.m_outBuffer.size() - 12;
                copy_n(inPtr, dataSize, flushIterator);
                flushIterator += dataSize;

                m_nextID = ++chunk.m_chunkID;
            }
            else{//the chunk is bad, it has a lousy checksum. Just go on.
                fprintf(stderr, "DecOutputQueue: chunk checksum is incorrect!");
            }
        }
    }
    m_flushSize = flushIterator - m_flushBuffer.begin();
    m_chunksLoaded = 0;
    return OK;
}

int32_t OutputQueue::Flush(){
    cerr << "Flush size = " << m_flushSize << endl;
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
