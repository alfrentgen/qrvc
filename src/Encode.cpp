#include "Encode.h"
#include <chrono>
#include "utilities.h"

#define DONT_DROP_TAIL false

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
            int32_t loaded = m_inQ->Load(DONT_DROP_TAIL);
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

        vector<uint8_t>& inChunk = m_data.m_inBuffer;
        vector<uint8_t>& rawFrame = m_data.m_outBuffer;
        uint8_t arrFrameID[8] = {0};
        uint8_t arrHashSum[4] = {0};
        //uint8_t* pInData = m_data.m_inBuffer.data();

        //put frame ID in data to encode little endian(lesser byte first)
        for(int i = 0; i < 8; i++){
            int32_t shift = 8 * i;
            arrFrameID[i] = (uint8_t)((m_data.m_chunkID >> shift) & (uint64_t)0xff);
        }
        inChunk.insert(inChunk.begin(), arrFrameID, arrFrameID + sizeof(arrFrameID));

        //calc and put hashsum in data to encode little endian(lesser byte first)
        uint32_t hashsum = m_data.CalcHashsum(inChunk.data(), inChunk.size());
        for(int i = 0; i < 4; i++){
            int32_t shift = 8 * i;
            arrHashSum[i] = (uint8_t)((hashsum >> shift) & (uint32_t)0xff);
        }
        inChunk.insert(inChunk.end(), arrHashSum, arrHashSum + sizeof(arrHashSum));

        //encode data
        QRcode* pQR = QRcode_encodeData(inChunk.size(), (unsigned char*)inChunk.data(), m_version, m_eccLevel);

        rawFrame.resize(m_frameWidth * m_frameHeight);
        //currently only black on white codes are supported
        rawFrame.assign(rawFrame.size(), 255);

        //put scaled code in the center of rawFrame
        //positioning
        uint8_t* pQRData = pQR->data;
        int32_t qrWidth = pQR->width;
        uint32_t xOffset = (m_frameWidth - m_qrScale * qrWidth)/2;
        uint32_t yOffset = (m_frameHeight - m_qrScale * qrWidth)/2;
        vector<uint8_t>::iterator frameIt = rawFrame.begin() + yOffset * m_frameWidth;

        for(int32_t y = 0; y < qrWidth; y++){
            for(int32_t x = 0; x < qrWidth; x++){
                uint8_t val = (0x01 & pQRData[x + y * qrWidth]) ? 0 : 255;
                fill_n(frameIt + xOffset + x * m_qrScale, m_qrScale, val);
            }
            frameIt += m_frameWidth;
            for(uint32_t cnt = 1; cnt < m_qrScale; ++cnt){
                copy_n(frameIt - m_frameWidth, m_frameWidth, frameIt);
                frameIt += m_frameWidth;
            }
        }
    return 0;
}

