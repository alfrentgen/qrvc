#include "Encode.h"
#include <chrono>
#include "utilities.h"

using namespace std;

static int32_t g_idCounter = 0;

Encode::Encode(int32_t fWidth, int32_t fHeight, InputQueue* inQ, OutputQueue* outQ):
    m_frameWidth(fWidth), m_frameHeight(fHeight), m_inQ(inQ), m_outQ(outQ), m_data(fWidth * fHeight),
    m_isWorking(true)
{
    //ctor
    m_ID = g_idCounter++;
}

Encode::~Encode()
{
    //dtor
}

static auto tp1 = chrono::steady_clock::now();
static auto tp2 = chrono::steady_clock::now();

int32_t Encode::Do(){
    int32_t result;
    unique_lock<mutex> lckInQ(m_inQ->m_syncMtx, defer_lock);
    unique_lock<mutex> lckOutQ(m_outQ->m_syncMtx, defer_lock);

    m_isWorking.test_and_set();
    while(m_isWorking.test_and_set()){
        //m_t1 = chrono::steady_clock::now();
        lckInQ.lock();
        while(m_inQ->m_waitForFlush){
            m_inQ->m_cv.wait(lckInQ);
        }

        //m_t2 = chrono::steady_clock::now();
        result = m_inQ->GetChunk(m_data);
        /*long long delta = chrono::duration_cast<chrono::microseconds>(m_t2 - m_t1).count();
        if(delta > 0){
            LOG("Job #%d get chunk wait time: %d\n", m_ID, delta);
        }*/

        if(result == INQ_EMPTY_AND_DEPLETED){
            return 0;
        }else
        if(result == INQ_EMPTY){
            int32_t loaded = m_inQ->Load();
            LOG("Job #%d is loading queue... %d frame(s) has been loaded.\n", m_ID, loaded);

            m_inQ->m_waitForFlush = true;
            lckInQ.unlock();

            lckOutQ.lock();
            //start new decode cycle if out queue is empty and flushed
            while(!m_outQ->m_flushed){  //out queue must always be constructed with flushed flag set
                m_outQ->m_cv.wait(lckOutQ);    //in the very beginning
            }
            m_outQ->m_flushed = false;
            lckOutQ.unlock();

            lckInQ.lock();
            //out queue capacity must always be equal to number of available chunks
            //in the input queue at the beginning of the new cycle
            lckOutQ.lock();
            m_outQ->SetCapacity(m_inQ->m_chunksAvailable);
            lckOutQ.unlock();

            m_inQ->m_waitForFlush = false;
            m_inQ->m_cv.notify_all();
            lckInQ.unlock();

            tp1 = chrono::steady_clock::now();

            continue;
        }
        lckInQ.unlock();

        //MEASURE_OPTIME(milliseconds,
        int32_t decRes;

        EncodeData();//);

        //START_TIME_MEASURING;
        //m_t1 = chrono::steady_clock::now();
        lckOutQ.lock();
        /*m_t2 = chrono::steady_clock::now();
        delta = chrono::duration_cast<chrono::microseconds>(m_t2 - m_t1).count();
        if(delta > 0){
            LOG("Job #%d put chunk wait time: %d\n", m_ID, delta);
        }*/

        m_outQ->Put(m_data);
        if(m_outQ->IsFull()){
            tp2 = chrono::steady_clock::now();
            LOG("Time of decode stage is: %d\n", chrono::duration_cast<chrono::milliseconds>(tp2 - tp1).count());

            m_outQ->PrepareFlush();
            {
                lock_guard<std::mutex> lck (m_outQ->m_flushMtx);
                m_outQ->m_flushed = true;
                m_outQ->m_cv.notify_all();
                lckOutQ.unlock();

                m_outQ->Flush();
            }
        }else{
            lckOutQ.unlock();
        }
    }
    m_isWorking.clear();
    return 0;
}

void Encode::Stop(){
    m_isWorking.clear();
}

uint32_t Encode::EncodeData(){
    return 0;
}

