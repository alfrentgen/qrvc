#include "inputQueue.h"

using namespace std;

InputQueue::InputQueue(istream* is, int32_t capacity, int32_t chunkSize):
m_chunksAvailable(0), m_depleted(false), m_frameCounter(0), m_syncMtx(), m_waitForFlush(false),
m_inStream(is)
{
    //ctor
    m_capacity = capacity <= 0 ? 2 : capacity;

    m_storage = vector<Chunk>(m_capacity, Chunk(chunkSize, 255));
    m_queue = deque<Chunk*>(0);
    m_chunkSize = chunkSize;
}

InputQueue::~InputQueue()
{
    //dtor
    if(m_inStream != NULL && m_inStream != &cin){
        try{
            ((ifstream*)m_inStream)->close();
        }catch(...){
        }
    }
}

int32_t InputQueue::Load(bool dropTail){
     while(m_chunksAvailable < m_capacity && !m_depleted){
        int32_t bytesRead = 0;
        vector<uint8_t>& buffer = m_storage[m_chunksAvailable].GetInBuffer();
        buffer.resize(m_chunkSize);//just for sure
        m_storage[m_chunksAvailable].Init();

        m_inStream->read((char*)buffer.data(), m_chunkSize);
        bytesRead = m_inStream->gcount();
        //try to read insufficient bytes
        if(bytesRead < m_chunkSize){
            while(m_inStream->rdstate() == ios_base::goodbit && bytesRead != m_chunkSize){
                m_inStream->read((char*)buffer.data() + bytesRead, m_chunkSize - bytesRead);
                bytesRead += m_inStream->gcount();
            }
        }

        if(m_inStream->rdstate() == ios::goodbit){
            m_storage[m_chunksAvailable].m_frameID = m_frameCounter++;
            m_queue.push_back(&m_storage[m_chunksAvailable]);
            ++m_chunksAvailable;
        }else {
            if(dropTail){
                //input stream is broken
                if(bytesRead == m_chunkSize){
                    m_storage[m_chunksAvailable].m_frameID = m_frameCounter++;
                    m_queue.push_back(&m_storage[m_chunksAvailable]);
                    ++m_chunksAvailable;
                }else{
                    m_storage[m_chunksAvailable].FreeInBuffer();
                    m_storage[m_chunksAvailable].FreeOutBuffer();
                }
                m_capacity = m_chunksAvailable;
                m_storage.resize(m_chunksAvailable);
                m_depleted = true;
                break;
            }else {
                m_storage[m_chunksAvailable].m_frameID = m_frameCounter++;
                m_storage[m_chunksAvailable].m_inBuffer.resize(bytesRead);
                m_queue.push_back(&m_storage[m_chunksAvailable]);
                ++m_chunksAvailable;
                m_capacity = m_chunksAvailable;
                m_storage.resize(m_chunksAvailable);
                m_depleted = true;
                break;
            }
        }
    }
    return m_chunksAvailable;
}

int32_t InputQueue::GetChunk(Chunk& chunkTo){
    if(m_chunksAvailable > 0){
        Chunk& chunkFrom = *m_queue.front();
        chunkTo.Init();

        swap(chunkTo.GetInBuffer(), chunkFrom.GetInBuffer());
        chunkTo.m_frameID = chunkFrom.m_frameID;
        m_queue.pop_front();

        --m_chunksAvailable;
        return m_chunksAvailable;
    }else if(m_depleted){
            return INQ_EMPTY_AND_DEPLETED;
    } else {
        return INQ_EMPTY;
    }
}

int32_t InputQueue::GetState(){
    int32_t result;
    if(m_chunksAvailable > 0){
        result = m_chunksAvailable;
    }else if(m_depleted){
        result = INQ_EMPTY_AND_DEPLETED;
    }else{
        result = INQ_EMPTY;
    }

    return result;
}
