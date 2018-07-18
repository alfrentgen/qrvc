#include "Encode.h"
#include <chrono>
#include "utilities.h"

using namespace std;

static int32_t g_idCounter = 0;

#define BLACK 0x0
#define WHITE 0xff

Encode::Encode(Config& config, InputQueue* inQ, OutputQueue* outQ):
    m_frameWidth(config.m_frameWidth), m_frameHeight(config.m_frameHeight), m_frameRepeats(config.m_frameRepeats),
    m_tailSize(config.m_nTrailingFrames), m_invertColors(config.m_inverseFrame),
    m_inQ(inQ), m_outQ(outQ), m_data(config.m_frameWidth * config.m_frameHeight),
    m_version(config.m_qrVersion), m_eccLevel(config.m_eccLevel), m_qrScale(config.m_qrScale), m_alignment(config.m_alignment),
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
            return OK;
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

        if(m_stegModule){
            ReadStegData();
            lckInQ.unlock();

            EncodeStegData();
        }else
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
    return OK;
}

void Encode::Stop(){
    m_isWorking.clear();
}

void putFrameID(Chunk& chunk){
    vector<uint8_t>& inBuffer = chunk.m_inBuffer;
    uint8_t arrFrameID[8] = {0};
    for(int i = 0; i < 8; i++){
        int32_t shift = 8 * i;
        arrFrameID[i] = (uint8_t)((chunk.m_frameID >> shift) & (uint64_t)0xff);
    }
    inBuffer.insert(inBuffer.begin(), arrFrameID, arrFrameID + sizeof(arrFrameID));
    return;
}

void putHashsum(Chunk& chunk){
    vector<uint8_t>& inBuffer = chunk.m_inBuffer;
    uint8_t arrHashSum[4] = {0};
    uint32_t hashsum = chunk.CalcHashsum(inBuffer.data(), inBuffer.size());
    for(int i = 0; i < 4; i++){
        int32_t shift = 8 * i;
        arrHashSum[i] = (uint8_t)((hashsum >> shift) & (uint32_t)0xff);
    }
    inBuffer.insert(inBuffer.end(), arrHashSum, arrHashSum + sizeof(arrHashSum));
    return;
}

uint32_t Encode::EncodeData(){
    vector<uint8_t>& inBuffer = m_data.m_inBuffer;
    vector<uint8_t>& rawFrame = m_data.m_outBuffer;

    uint8_t color = m_invertColors ? WHITE : BLACK;
    int32_t frameSize = m_frameWidth * m_frameHeight;

    //put frame ID in data to encode little endian(lesser byte first)
    putFrameID(m_data);

    //calc and put hashsum in data to encode little endian(lesser byte first)
    putHashsum(m_data);

    //encode data
    QRcode* pQR = QRcode_encodeData(inBuffer.size(), (unsigned char*)inBuffer.data(), m_version, m_eccLevel);
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
                m_pKeyFileStream->flush();
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
    return OK;
}

void Encode::SetCypheringParams(vector<uint8_t>* pKeyFrame, ofstream* pKeyFileOS){
    m_pKey = pKeyFrame;
    m_pKeyFileStream = pKeyFileOS;
}

uint32_t Encode::EncodeStegData(){
    vector<uint8_t>& inBuffer = m_data.m_inBuffer;
    //put frame ID in data to encode little endian(lesser byte first)
    putFrameID(m_data);
    //calc and put hashsum in data to encode little endian(lesser byte first)
    putHashsum(m_data);

    //encode data
    QRcode* pQR = QRcode_encodeData(inBuffer.size(), (unsigned char*)inBuffer.data(), m_version, m_eccLevel);
    uint8_t* pQRData = pQR->data;
    int32_t qrWidth = pQR->width;

    int32_t frameSize = m_frameWidth * m_frameHeight;
    frameSize += frameSize/2;//YUV420, add 1/4 for U and 1/4 for V planes
    uint8_t* pRawFrame = m_data.m_outBuffer.data();
    for(int i = 0; i < m_frameRepeats; i++){
        //m_stegModule->Hide(pRawFrame, pQRData);
        m_stegModule->Process(pRawFrame, pQRData, STEG_HIDE);
        pRawFrame += frameSize;
    }
    QRcode_free(pQR);
    return OK;
}

uint32_t Encode::ReadStegData(){
    vector<uint8_t>& rawFrame = m_data.m_outBuffer;
    int32_t frameSize = m_frameWidth * m_frameHeight;
    frameSize += frameSize/2;//YUV420, add 1/4 for U and 1/4 for V planes
    rawFrame.resize(frameSize * m_frameRepeats, 128);
    cin.read(rawFrame.data(), frameSize * m_frameRepeats);
    if(cin.bad()){
        return FAIL;
    }
    return OK;
}

void Encode::SetStegParams(StegModule* pStegModule){
    m_stegModule = pStegModule;
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
