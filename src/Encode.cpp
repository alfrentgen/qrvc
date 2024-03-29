#include "Encode.h"
#include <chrono>
#include "utilities.h"

using namespace std;

static int32_t g_idCounter = 0;

#define BLACK 0x0
#define WHITE 0xff

Encode::Encode(int32_t fWidth, int32_t fHeight, int32_t frameRepeats, int32_t tailSize, bool invert,
                InputQueue* inQ, OutputQueue* outQ,
                int32_t version, QRecLevel eccLevel, int32_t qrScale, int32_t alignment):
    m_frameWidth(fWidth), m_frameHeight(fHeight), m_frameRepeats(frameRepeats),
    m_tailSize(tailSize), m_invertColors(invert),
    m_inQ(inQ), m_outQ(outQ), m_data(fWidth * fHeight),
    m_version(version), m_eccLevel(eccLevel), m_qrScale(qrScale), m_alignment(alignment),
    m_isWorking(false), m_pKeyFileStream(NULL), m_pKey(NULL)
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
        lckInQ.lock();
        while(m_inQ->m_waitForFlush){
            m_inQ->m_cv.wait(lckInQ);
        }

        result = m_inQ->GetChunk(m_data);
        if(result == 0 && m_inQ->GetState() == INQ_EMPTY_AND_DEPLETED){
            m_frameRepeats += m_tailSize;
        }

        if(result == INQ_EMPTY_AND_DEPLETED){
            lckInQ.unlock();
            return 0;
        }else
        if(result == INQ_EMPTY){
            int32_t loaded = m_inQ->Load(!DROP_TAIL);
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
            continue;
        }

        if(m_data.m_frameID == 0 && m_pKey){
            EncodeData();
            lckInQ.unlock();
        }else{
            lckInQ.unlock();
            EncodeData();
        }

        lckOutQ.lock();
        m_outQ->Put(m_data);
        if(m_outQ->IsFull()){
            m_outQ->PrepareFlush(SIMPLE_FLUSH);
            m_outQ->Flush();
            m_outQ->m_flushed = true;
            m_outQ->m_cv.notify_all();
        }
        lckOutQ.unlock();
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
    uint8_t color = m_invertColors ? WHITE : BLACK;
    int32_t frameSize = m_frameWidth * m_frameHeight;

    //put frame ID in data to encode little endian(lesser byte first)
    for(int i = 0; i < 8; i++){
        int32_t shift = 8 * i;
        arrFrameID[i] = (uint8_t)((m_data.m_frameID >> shift) & (uint64_t)0xff);
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

    uint8_t* pQRData = pQR->data;
    int32_t qrWidth = pQR->width;
    uint32_t xOffset;
    uint32_t yOffset;

    if(m_alignment){
        int32_t nPadsX = (m_frameWidth - m_qrScale * qrWidth) / m_alignment;
        int32_t nPadsY = (m_frameHeight - m_qrScale * qrWidth) / m_alignment;

        if(nPadsX == 1){
            xOffset = m_alignment;
        }else{
            xOffset = (nPadsX/2) * m_alignment;
        }

        if(nPadsY == 1){
            yOffset = m_alignment;
        }else{
            yOffset = (nPadsY/2) * m_alignment;
        }

    } else{
        xOffset = (m_frameWidth - m_qrScale * qrWidth)/2;
        yOffset = (m_frameHeight - m_qrScale * qrWidth)/2;
    }

    if(m_pKey){
        if(m_data.m_frameID == 0){
            m_pKey->resize(qrWidth * qrWidth);
            m_pKey->assign(pQRData, pQRData + qrWidth * qrWidth);

            //if a separate key file must be written, we make the first code blank for getting first frames blank.
            if(m_pKeyFileStream && m_pKeyFileStream->is_open()){
                //write key frame to file
                vector<uint8_t> keyFrame(frameSize);
                keyFrame.assign(keyFrame.size(), ~color);
                FillFrames(keyFrame, frameSize, xOffset, yOffset, 0, pQRData, qrWidth, color, ~color);
                m_pKeyFileStream->write((char*)keyFrame.data(), frameSize);
                m_pKeyFileStream->close();

                fill_n(pQRData, qrWidth * qrWidth, 0);
            }
        }else{
            for(int i = 0 ; i < qrWidth * qrWidth; i++){
                pQRData[i] ^= (*m_pKey)[i];
            }
#ifdef MOAR_COMPRESSION
            //mark phase change with a black dot, if the phase is unchanged draw white then
            /*for(int i = 0 ; i < qrWidth; i++){
                uint8_t prevPhase = (pQRData[i * qrWidth] == (*m_pKey)[i * qrWidth]) ? 0 : 1;//0 - match, 1 - unmatch;
                pQRData[i * qrWidth] = prevPhase;
                for(int j = 1 ; j < qrWidth; j++){
                    uint8_t curPhase = (pQRData[i * qrWidth + j] == (*m_pKey)[i * qrWidth + j]) ? 0 : 1;
                    if(curPhase == prevPhase){
                        pQRData[i * qrWidth + j] = 0;
                    }else{
                        pQRData[i * qrWidth + j] = 1;
                        prevPhase = curPhase;
                    }
                }
            }*/

            //draw color is inversed on phase switch
            /*for(int i = 0 ; i < qrWidth; i++){
                uint8_t prevPhase = (pQRData[i * qrWidth] == (*m_pKey)[i * qrWidth]) ? 0 : 1;//0 - match, 1 - unmatch;
                uint8_t color = prevPhase;
                pQRData[i * qrWidth] = color;
                for(int j = 1 ; j < qrWidth; j++){
                    uint8_t curPhase = (pQRData[i * qrWidth + j] == (*m_pKey)[i * qrWidth + j]) ? 0 : 1;
                    if(curPhase == prevPhase){
                        pQRData[i * qrWidth + j] = color;
                    }else{
                        color = color ? 0 : 1;
                        pQRData[i * qrWidth + j] = color;
                        prevPhase = curPhase;
                    }
                }
            }*/
#endif
        }
    }

    rawFrame.resize(m_frameRepeats * frameSize);
    rawFrame.assign(rawFrame.size(), ~color);
    FillFrames(rawFrame, frameSize, xOffset, yOffset, m_frameRepeats, pQRData, qrWidth, color, ~color);

    //mem leakage is unwanted
    QRcode_free(pQR);
    return 0;
}

void Encode::SetCypheringParams(vector<uint8_t>* pKeyFrame, ofstream* pKeyFileOS){
    m_pKey = pKeyFrame;
    m_pKeyFileStream = pKeyFileOS;
}

void Encode::FillFrames(vector<uint8_t>& frames, int32_t frameSize, int32_t xOffset, int32_t yOffset, int32_t frameRepeats,
                        uint8_t* pQRData ,int32_t qrWidth, int32_t drawColor, int32_t bgColor){

    vector<uint8_t>::iterator frameIt = frames.begin() + yOffset * m_frameWidth;

    for(int32_t y = 0; y < qrWidth; y++){
        for(int32_t x = 0; x < qrWidth; x++){
            uint8_t val = (0x01 & pQRData[x + y * qrWidth]) ? drawColor : bgColor;
            fill_n(frameIt + xOffset + x * m_qrScale, m_qrScale, val);
        }
        frameIt += m_frameWidth;
        for(uint32_t cnt = 1; cnt < m_qrScale; ++cnt){
            copy_n(frameIt - m_frameWidth, m_frameWidth, frameIt);
            frameIt += m_frameWidth;
        }
    }

    frameIt = frames.begin();
    for(int i = 1; i < m_frameRepeats; i++){
        frameIt += frameSize;
        copy_n(frames.begin(), frameSize, frameIt);
    }
}
