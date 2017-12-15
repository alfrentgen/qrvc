#include "Decode.h"
#include <chrono>
#include "utilities.h"

using namespace std;
using namespace zbar;

static int32_t g_idCounter = 0;

Decode::Decode(int32_t fWidth, int32_t fHeight, InputQueue* inQ, OutputQueue* outQ, DecodeMode decMode):
    m_frameWidth(fWidth), m_frameHeight(fHeight), m_inQ(inQ), m_outQ(outQ), m_data(fWidth * fHeight),
    m_image(fWidth, fHeight, string("GREY"), NULL, fWidth * fHeight), m_isWorking(true), m_decMode(decMode)
{
    //ctor
    m_scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);

    m_qr = quirc_new();
    if (!m_qr) {
	    perror("Failed to allocate memory");
	    abort();
    }
    if (quirc_resize(m_qr, fWidth, fHeight) < 0) {
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
        lckInQ.unlock();

        int32_t decRes;
        if(m_decMode == QUICK){
            decRes = DecodeDataQuick();
        }else if(m_decMode == SLOW){
            decRes = DecodeData();
        }else{
            decRes = DecodeDataQuick();
            if(decRes){
                decRes = DecodeData();
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
        LOG("Decoding chunk #%llu, errors (code = %d) have occured during decoding!\n", m_data.m_frameID, decodeResult);
        m_data.m_rendered = false;
        return -1;
    }else if(decodeResult == 0){
        LOG("Decoding chunk #%llu, decoded %d symbols. Nothing was decoded!\n", m_data.m_frameID, decodeResult);
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
    m_data.m_chunkID = 0;
    for(int i = 0; i < 8; i++){
        int32_t shift = 8 * i;
        m_data.m_chunkID |= ((uint64_t)m_data.m_outBuffer[i]) << shift;
    }

    //extracting hashsum
    m_data.m_hashsum = 0;
    uint8_t* outBuffer = m_data.m_outBuffer.data() + m_data.m_outBuffer.size() - 4;
    for(int i = 0; i < 4; i++){
        int32_t shift = 8 * i;
        m_data.m_hashsum |= (uint32_t)outBuffer[i] << shift;
    }

    m_data.m_rendered = true;
    if(decodedData.size() == 12 ){
        LOG("Decoded data size %llu bytes. Nothing to write out! Will be skipped.\n", decodedData.size());
        m_data.m_rendered = false;
    }

    return 0;
}

uint32_t Decode::DecodeDataQuick(){
    uint8_t* pImage = quirc_begin(m_qr, &m_frameWidth, &m_frameHeight);
    m_qr->image = m_data.m_inBuffer.data();
    quirc_end(m_qr);
    int32_t num_codes = quirc_count(m_qr);

    if(num_codes == 0){
        LOG("Decoding chunk #%llu, decoded %d symbols. Nothing was decoded!\n", m_data.m_frameID, num_codes);
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
		    return -1;
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
        return -1;
    }

    //extracting chunk ID
    m_data.m_chunkID = 0;
    for(int i = 0; i < 8; i++){
        int32_t shift = 8 * i;
        m_data.m_chunkID |= ((uint64_t)m_data.m_outBuffer[i]) << shift;
    }

    //extracting hashsum
    m_data.m_hashsum = 0;
    uint8_t* outBuffer = m_data.m_outBuffer.data() + m_data.m_outBuffer.size() - 4;
    for(int i = 0; i < 4; i++){
        int32_t shift = 8 * i;
        m_data.m_hashsum |= (uint32_t)outBuffer[i] << shift;
    }

    m_data.m_rendered = true;
    if(m_data.m_outBuffer.size() == 12 ){
        LOG("Decoded data size %llu bytes. Nothing to write out! Will be skipped.\n", m_data.m_outBuffer.size());
        m_data.m_rendered = false;
    }

    return 0;
}

/*
uint32_t Decode::DecodeData_mock(){
    cerr << "In DecodeData().\n";
    m_data.m_inBuffer.swap(m_data.m_outBuffer);
    m_data.m_rendered = true;
    return 0;
}
*/

/*DecodeQ::DecodeQ(int32_t fWidth, int32_t fHeight, InputQueue* inQ, OutputQueue* outQ)
: Decode(fWidth, fHeight, inQ, outQ)
{
    //ctor
    //Decode(fWidth, fHeight, inQ, outQ);
    //m_scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
    m_qr = quirc_new();
    if (!m_qr) {
	    perror("Failed to allocate memory");
	    abort();
    }
    if (quirc_resize(m_qr, fWidth, fHeight) < 0) {
	    perror("Failed to allocate video memory");
	    abort();
    }
}

uint32_t DecodeQ::DecodeData(){
    //cerr << "In Decode::DecodeData():\n";
    //cerr << "m_data.m_frameID = " << m_data.m_frameID << endl;

    uint8_t* pImage = quirc_begin(m_qr, &m_frameWidth, &m_frameHeight);
    m_qr->image = m_data.m_inBuffer.data();
    quirc_end(m_qr);
    int32_t num_codes = quirc_count(m_qr);

    if(num_codes == 0){
        fprintf(stderr, "Decoding chunk #%llu, decoded %d symbols. Nothing was decoded!\n", m_data.m_frameID, num_codes);
        m_data.m_rendered = false;
        return -1;
    }

    m_data.m_outBuffer.clear();
    for (int32_t i = 0; i < num_codes; i++) {
	    struct quirc_code code;
	    struct quirc_data data;
	    quirc_decode_error_t err;

	    quirc_extract(m_qr, i, &code);

	    // Decoding stage
	    err = quirc_decode(&code, &data);
	    if (err){
		    fprintf(stderr, "DECODE FAILED: %s\n", quirc_strerror(err));
	    }
	    else{
		    //fprintf(stderr, "Data: %s\n", data.payload);
		    int32_t outBufferSize = m_data.m_outBuffer.size();
            m_data.m_outBuffer.resize(outBufferSize + data.payload_len);
            uint8_t* curPosition = m_data.m_outBuffer.data() + outBufferSize;
            copy_n(data.payload, data.payload_len, curPosition);
	    }
    }

    if(m_data.m_outBuffer.size() < 12 ){
        fprintf(stderr, "Decoded data size %llu < 12 bytes. Not enough to get checksum and frame ID.\n", m_data.m_outBuffer.size());
        m_data.m_rendered = false;
        return -1;
    }

    //extracting chunk ID
    m_data.m_chunkID = 0;
    for(int i = 0; i < 8; i++){
        int32_t shift = 8 * i;
        m_data.m_chunkID |= ((uint64_t)m_data.m_outBuffer[i]) << shift;
    }

    //extracting hashsum
    m_data.m_hashsum = 0;
    uint8_t* outBuffer = m_data.m_outBuffer.data() + m_data.m_outBuffer.size() - 4;
    for(int i = 0; i < 4; i++){
        int32_t shift = 8 * i;
        m_data.m_hashsum |= (uint32_t)outBuffer[i] << shift;
    }

    if(m_data.m_outBuffer.size() == 12 ){
        fprintf(stderr, "Decoded data size %llu bytes. Nothing to write out! Will be skipped.\n", m_data.m_outBuffer.size());
        m_data.m_rendered = false;
    }

    m_data.m_rendered = true;

    return 0;
}

DecodeQ::~DecodeQ()
{
    //dtor
    quirc_destroy(m_qr);
}
*/
