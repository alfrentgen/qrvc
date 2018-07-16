#include "Decode.h"
#include <chrono>
#include "utilities.h"

using namespace std;
using namespace zbar;

static int32_t g_idCounter = 0;

Decode::Decode(Config& config, InputQueue* inQ, OutputQueue* outQ):
    m_frameWidth(config.m_frameWidth), m_frameHeight(config.m_frameHeight), m_inQ(inQ), m_outQ(outQ),
    m_data(config.m_frameWidth * config.m_frameHeight),
    m_image(config.m_frameWidth, config.m_frameHeight, string("GREY"), NULL, config.m_frameWidth * config.m_frameHeight),
    m_isWorking(true), m_decMode(config.m_decMode), m_skipDup(config.m_skipDupFrames)
{
    //ctor
    m_scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);

    m_qr = quirc_new();
    if (!m_qr) {
	    perror("Failed to allocate memory");
	    abort();
    }
    if (quirc_resize(m_qr, config.m_frameWidth, config.m_frameHeight) < 0) {
	    perror("Failed to allocate video memory");
	    abort();
    }
    m_ID = g_idCounter++;
}

Decode::~Decode()
{
    //dtor
    quirc_destroy(m_qr);
}

static auto tp1 = chrono::steady_clock::now();
static auto tp2 = chrono::steady_clock::now();

int32_t Decode::Do(){
    int32_t result;
    vector<Chunk*> snapshot(0);
    unique_lock<mutex> lckInQ(m_inQ->m_syncMtx, defer_lock);
    unique_lock<mutex> lckOutQ(m_outQ->m_syncMtx, defer_lock);

    m_isWorking.test_and_set();
    while(m_isWorking.test_and_set()){

        lckInQ.lock();
        while(m_inQ->m_waitForFlush){
            m_inQ->m_cv.wait(lckInQ);
        }
        result = m_inQ->GetChunk(m_data);

        if(result == INQ_EMPTY_AND_DEPLETED){
            lckInQ.unlock();
            return 0;
        }else
        if(result == INQ_EMPTY){
            int32_t loaded = m_inQ->Load(DROP_TAIL);
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
        //save zero frame as key frame if m_pKeyFrame was given and not filled.
        if(m_data.m_frameID == 0 && m_pKeyFrame && m_pKeyFrame->size() == 0){
            //save
            m_pKeyFrame->assign(m_data.m_inBuffer.begin(), m_data.m_inBuffer.end());
            //denoise
            for(int32_t i = 0; i < m_pKeyFrame->size(); i++){
                (*m_pKeyFrame)[i] = ( (*m_pKeyFrame)[i] > 127 ) ? 255 : 0;
            }
            LOG("Reading zero frame, size = %d.\n", m_data.m_inBuffer.size());
            lckInQ.unlock();
            goto skipDecyph;//yeah!
        }
        lckInQ.unlock();

        //decypher if key frame was given
        if(m_pKeyFrame){

#ifdef MOAR_COMPRESSION
            for(int32_t j = 0; j < m_frameHeight; j++){
                bool match = true;
                if(m_data.m_inBuffer[j * m_frameWidth] == 0){
                    match = false
                }

                int32_t blockSize = 0;
                uint8_t* position = &m_data.m_inBuffer[j * m_frameWidth];
                for(int32_t i = 1; i < m_frameWidth; i++){
                    //white to black transition triggers phase change
                    if(m_data.m_inBuffer[j * m_frameWidth + i - 1] == 255 && m_data.m_inBuffer[j * m_frameWidth + i] == 0){
                        match = !match;
                        //calcuate block size
                        while(){
                        }
                        //prevValue = m_data.m_inBuffer[j * m_frameWidth + i - 1];
                        break;
                    }
                }
            }

            for(int32_t i = 0; i < m_frameWidth * m_frameHeight; i++){
                m_data.m_inBuffer[i] = (m_data.m_inBuffer[i] > 127) ? 255 : 0;
                m_data.m_inBuffer[i] ^= ~((*m_pKeyFrame)[i]);
            }
#endif
            for(int32_t i = 0; i < m_frameWidth * m_frameHeight; i++){
                m_data.m_inBuffer[i] = (m_data.m_inBuffer[i] > 127) ? 255 : 0;
                m_data.m_inBuffer[i] ^= ~((*m_pKeyFrame)[i]);
            }
        }

skipDecyph:
        //check if the frame  already has a double in output queue
        m_data.m_inHash = m_data.CalcHashsum(m_data.m_inBuffer.data(), m_data.m_inBuffer.size());

        bool duplicated = false;
        if(m_skipDup){
            lckOutQ.lock();
            int32_t nChunks = m_outQ->GetSnapshot(snapshot);
            lckOutQ.unlock();

            for(int i= 0; i < nChunks; i++){
                if(snapshot[i]->m_inHash == m_data.m_inHash){
                    if(snapshot[i]->m_inBuffer == m_data.m_inBuffer){
                        duplicated = true;
                    }
                }
            }
        }

        if(m_skipDup && duplicated){
            m_data.m_rendered = false;
        }else{
            int32_t decRes;
#define STEG_DECODE
#ifdef STEG_DECODE
            if(m_stegModule){
                vector<uint8_t>& frame = m_data.m_inBuffer;
                int32_t qrWidth = m_stegModule->m_qrWidth;
                vector<uint8_t> qrCode(qrWidth * qrWidth);
                m_stegModule->Reveal(frame.data(), qrCode.data());
                DecodeDataSteg(qrCode.data(), qrWidth);
            }else
#endif
            if(m_decMode == MODE_QUICK){
                decRes = DecodeDataQuick();
            }else if(m_decMode == MODE_SLOW){
                decRes = DecodeData();
            }else{
                decRes = DecodeDataQuick();
                if(decRes){
                    decRes = DecodeData();
                }
            }

            if(decRes == OK){
                uint32_t hashsum = m_data.CalcHashsum(m_data.m_outBuffer.data(), m_data.m_outBuffer.size() - 4);
                if(hashsum != m_data.m_outHash){
                    m_data.m_rendered = false;
                    LOG("Decoded chunk checksum is incorrect! The chunk will be skipped.\n");
                }
            }
        }

        lckOutQ.lock();
        m_outQ->Put(m_data);
        if(m_outQ->IsFull()){
            m_outQ->PrepareFlush(!SIMPLE_FLUSH);
            m_outQ->Flush();
            m_outQ->m_flushed = true;
            m_outQ->m_cv.notify_all();
        }
        lckOutQ.unlock();
    }
    m_isWorking.clear();
    return 0;
}

void Decode::Stop(){
    m_isWorking.clear();
}

uint32_t Decode::DecodeData(){
    m_image.set_data((void*)m_data.m_inBuffer.data(), m_data.m_inBuffer.size());
    //MEASURE_OPTIME(milliseconds,
    int32_t decodeResult = m_scanner.scan(m_image);
    //);

    if(decodeResult < 0){
        LOG("SLOW: Decoding chunk #%llu, errors (code = %d) have occured during decoding!\n", m_data.m_frameID, decodeResult);
        m_data.m_rendered = false;
        return -1;
    }else if(decodeResult == 0){
        LOG("SLOW: Decoding chunk #%llu, decoded %d symbols. Nothing was decoded!\n", m_data.m_frameID, decodeResult);
        m_data.m_rendered = false;
        return -1;
    }

    string decodedData = string();
    //Iterate over all symbols!!! Not sure if it is correct, due to only one symbol(QR code) should be found.
    for(Image::SymbolIterator symbol = m_image.symbol_begin(); symbol != m_image.symbol_end(); ++symbol) {
        decodedData += symbol->get_data();
    }

    m_data.m_outBuffer.resize(decodedData.size());

    if(decodedData.size() < 12 ){
        LOG("Decoded data size %llu < 12 bytes. Not enough to get checksum and frame ID.\n", decodedData.size());
        //m_data.m_outBuffer.resize(decodedData.size());
        m_data.m_rendered = false;
        return -1;
    }

    decodedData.copy((char*)m_data.m_outBuffer.data(), decodedData.size());

    //extracting chunk ID
    m_data.m_chunkID = ExtractChunkID();

    //extracting hashsum
    m_data.m_outHash = ExtractHashsum();

    m_data.m_rendered = true;
    if(decodedData.size() == 12 ){
        LOG("Decoded data size %llu bytes. Nothing to write out! Will be skipped.\n", decodedData.size());
        m_data.m_rendered = false;
    }

    return 0;
}

uint32_t Decode::DecodeDataQuick(){
    //quirc_resize(m_qr, m_frameWidth, m_frameHeight);
    uint8_t* pImage = quirc_begin(m_qr, &m_frameWidth, &m_frameHeight);
    copy_n(m_data.m_inBuffer.data(), m_frameWidth * m_frameHeight, pImage);

    quirc_end(m_qr);
    int32_t num_codes = quirc_count(m_qr);
    //LOG("num_codes=%d\n", num_codes);

    if(num_codes == 0){
        LOG("QUICK: Decoding chunk #%llu, decoded %d symbols. Nothing was decoded!\n", m_data.m_frameID, num_codes);
        m_data.m_rendered = false;
        return -1;
    }

    m_data.m_outBuffer.clear();
    for (int32_t i = 0; i < num_codes; i++) {
	    struct quirc_code code;
	    struct quirc_data data;
	    quirc_decode_error_t err;

	    quirc_extract(m_qr, i, &code);

	    /* Decoding stage */
	    err = quirc_decode(&code, &data);
	    if (err){
		    LOG("Quick decode failed: %s\n", quirc_strerror(err));
		    return FAIL;
	    }
	    else{
		    int32_t outBufferSize = m_data.m_outBuffer.size();
            m_data.m_outBuffer.resize(outBufferSize + data.payload_len);
            uint8_t* curPosition = m_data.m_outBuffer.data() + outBufferSize;
            copy_n(data.payload, data.payload_len, curPosition);
	    }
    }

    if(m_data.m_outBuffer.size() < 12 ){
        LOG("Decoded data size %llu < 12 bytes. Not enough to get checksum and frame ID.\n", m_data.m_outBuffer.size());
        m_data.m_rendered = false;
        return FAIL;
    }

    //extracting chunk ID
    m_data.m_chunkID = ExtractChunkID();

    //extracting hashsum
    m_data.m_outHash = ExtractHashsum();

    m_data.m_rendered = true;
    if(m_data.m_outBuffer.size() == 12 ){
        LOG("Decoded data size %llu bytes. Nothing to write out! Will be skipped.\n", m_data.m_outBuffer.size());
        m_data.m_rendered = false;
    }

    return OK;
}

uint32_t Decode::DecodeDataSteg(uint8_t* qrCode, int32_t size){
    quirc_code quircCode;
    struct quirc_data data;
    uint32_t decResult = OK;

    memset(&quircCode, 0, sizeof(quircCode));
    memset(&data, 0, sizeof(data));

    quircCode.size = size;
    /* The number of cells across in the QR-code. The cell bitmap
	 * is a bitmask giving the actual values of cells. If the cell
	 * at (x, y) is black, then the following bit is set:
	 *
	 *     cell_bitmap[i >> 3] & (1 << (i & 7))
	 *
	 * where i = (y * size) + x.
	 */
    for(int i = 0; i < size*size; i++){
        if((0x01 & qrCode[i])){
            quircCode.cell_bitmap[i >> 3] |= (1 << (i & 7));
        }
    }

    quirc_decode_error_t err = quirc_decode(&quircCode, &data);
    if (err){
        LOG("Quick decode failed: %s\n", quirc_strerror(err));
        return FAIL;
    }else{
        int32_t outBufferSize = m_data.m_outBuffer.size();
        m_data.m_outBuffer.assign(data.payload, data.payload + data.payload_len);
    }

    if(m_data.m_outBuffer.size() < 12 ){
        LOG("Decoded data size %llu < 12 bytes. Not enough to get checksum and frame ID.\n", m_data.m_outBuffer.size());
        m_data.m_rendered = false;
        decResult = FAIL;
    }

    //extracting chunk ID
    m_data.m_chunkID = ExtractChunkID();

    //extracting hashsum
    m_data.m_outHash = ExtractHashsum();

    m_data.m_rendered = true;
    if(m_data.m_outBuffer.size() == 12 ){
        LOG("Decoded data size %llu bytes. Nothing to write out! Will be skipped.\n", m_data.m_outBuffer.size());
        m_data.m_rendered = false;
    }

    return decResult;
}

uint32_t Decode::ExtractHashsum(){
    //extracting hashsum
    uint32_t hashsum = 0;
    uint8_t* outBuffer = m_data.m_outBuffer.data() + m_data.m_outBuffer.size() - 4;
    for(int i = 0; i < 4; i++){
        int32_t shift = 8 * i;
        hashsum |= (uint32_t)outBuffer[i] << shift;
    }
    return hashsum;
}

uint64_t Decode::ExtractChunkID(){
    //extracting chunk ID
    uint64_t id = 0;
    for(int i = 0; i < 8; i++){
        int32_t shift = 8 * i;
        id |= ((uint64_t)m_data.m_outBuffer[i]) << shift;
    }
    return id;
}

void Decode::SetCypheringParams(vector<uint8_t>* pKeyFrame){
    m_pKeyFrame = pKeyFrame;
}

void Decode::SetStegParams(StegModule* pStegModule){
    m_stegModule = pStegModule;
}
