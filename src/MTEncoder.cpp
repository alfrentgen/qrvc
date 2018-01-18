#include <MTEncoder.h>
#include "utilities.h"

int estimateBitStreamSizeOfEntry(QRinput_List *entry, int version, int mqr)
{
	int bits = 0;
	int l, m;
	int num;

	if(version == 0) version = 1;

	bits = QRinput_estimateBitsMode8(entry->size);

	l = QRspec_lengthIndicator(entry->mode, version);
	m = 1 << l;
	num = (entry->size + m - 1) / m;
	bits += num * (MODE_INDICATOR_SIZE + l);

	return bits;
}

uint32_t getChunkSize(uint32_t frameWidth, uint32_t frameHeight, QRecLevel eccLevel, uint32_t qrScale, uint32_t *retVersion){
    uint32_t minFrameDim = frameWidth > frameHeight ? frameHeight : frameWidth;
    //resolve input chunk size automatically
    uint32_t version = QRSPEC_VERSION_MAX;
    while(qrScale * QRspec_getWidth(version) > minFrameDim && version != 0){
        --version;
    }
    if(version == 0){
        return 0;
    }

    QRinput_List mockList;
    mockList.mode = QR_MODE_8;
    mockList.size = MAX_BIN_CHUNKSIZE;
    uint32_t minVersion;
    do{
        uint32_t bits = estimateBitStreamSizeOfEntry(&mockList, version, NOT_MQR);
        minVersion = QRspec_getMinimumVersion((bits + 7) / 8, eccLevel);
        mockList.size--;
    }while(minVersion != version);

    *retVersion = version;
    return mockList.size;
}

MTEncoder::MTEncoder()
{
    //ctor
}

MTEncoder::~MTEncoder()
{
    //dtor
    Stop();
}

int32_t MTEncoder::Init(Config& config){

    ifstream* ifs = NULL;
    ofstream* ofs = NULL;
    istream* inputStream = &cin;
    ostream* outputStream = &cout;

    if(config.m_ifName.size() == 0){
        cerr << "Input filename is not specified, reading from stdin.\n";
    }else{
        ifs = new ifstream(config.m_ifName, ios_base::in | ios_base::binary);
        if (!ifs || !ifs->is_open()){
            ifs = NULL;
            cerr << "Failed to open input stream.";
            return FAIL;
        }
        inputStream = ifs;
    }

    if(config.m_ofName.size() == 0){
        cerr << "Output filename is not specified, writing to stdout.\n";
    }else{
        ofs = new ofstream(config.m_ofName, ios_base::out | ios_base::binary);
        if (!ofs || !ofs->is_open()){
            ofs = NULL;
            cerr << "Failed to open output stream.";
            return FAIL;
        }
        outputStream = ofs;
    }

    if(config.m_eccLevel < 0 || config.m_eccLevel > 3){
        return FAIL;
    }

    Init(inputStream, outputStream,
        config.m_frameWidth, config.m_frameHeight, config.m_frameRepeats, config.m_nTrailingFrames, config.m_inverseFrame,
        config.m_eccLevel, config.m_qrScale, config.m_alignment, config.m_framesPerThread, config.m_nWorkingThreads);
    return OK;
}

int32_t MTEncoder::Init(istream* is, ostream* os,
                        int32_t frameWidth, int32_t frameHeight, int32_t frameRepeat, int32_t tailSize, bool invert,
                        QRecLevel eccLevel, int32_t qrScale, int32_t alignment, uint32_t framesPerThread, uint32_t nThreads){
    //check if frame size fits QR code size
    uint32_t version = 0;
    int32_t chunkSize = getChunkSize(frameWidth - alignment, frameHeight - alignment, eccLevel, qrScale, &version);
    if(!chunkSize || version > 40 || version <= 0){
        cerr << "Frame size does not fit any possible qr code. Try smaller scale, ECC  level, bigger frame or changing alignment.\n";
        return FAIL;
    }else{
        cerr << "QR version: " << version << endl;
    }
    m_qrVersion = version;

    int32_t nBytesToRead = chunkSize - COUNTER_SIZE - HASHSUM_SIZE;
    cerr << "Chunk size: " << chunkSize << endl;
    cerr << "Bytes to read: " << nBytesToRead << endl;

    //calculate number of threads and input queue size
    if(nThreads == 0){
        m_nThreads = std::thread::hardware_concurrency();
        m_nThreads = (m_nThreads == 0) ? 2 : m_nThreads;
    }else{
        m_nThreads = nThreads;
    }

    int32_t queueSize;
    if(framesPerThread == 0){
        framesPerThread = 8;
    }
    queueSize = framesPerThread * m_nThreads;

    m_inQ = new InputQueue(is, queueSize, nBytesToRead);
    m_outQ = new OutputQueue(os, queueSize, frameWidth * frameHeight);

    m_threads.clear();

    m_jobs.resize(m_nThreads);

    m_invertColors = invert;
    for(int i =0; i < m_nThreads; i++){
        m_jobs[i] = new Encode(frameWidth, frameHeight, frameRepeat, tailSize, invert,
                                m_inQ, m_outQ,
                                m_qrVersion, eccLevel, qrScale, alignment);
    }

    LOG("Number of working threads is: %d\n", m_nThreads);

    return OK;
}

int32_t MTEncoder::Start(bool join){

    m_threads.clear();
    try{
        LOG("Strating %d threads.\n", m_nThreads);
        for(int i = 0; i < m_nThreads; i++){
        LOG("Strating thread #%d.\n", i);
            m_threads.push_back(thread(&Encode::Do, m_jobs[i]));
        }

        if(join){
            for(int i = 0; i < m_nThreads; i++){
                if(m_threads[i].joinable()){
                    m_threads[i].join();
                }
            }
            m_threads.clear();
        }
    }catch(exception& e){
        cerr << e.what() << endl;
    }
    return OK;
}

int32_t MTEncoder::Stop(){
    for(int i = 0; i < m_nThreads; i++){
        m_jobs[i]->Stop();
    }
    for(int i = 0; i < m_threads.size(); i++){
        if(m_threads[i].joinable()){
            m_threads[i].join();
        }
    }

    m_threads.clear();

    //need to wait for threads to be stopped
    //and streams to be closed
    return 0;
}

int main(int argc, char** argv){
    ArgsParserDec ap = ArgsParserDec();
    if(ap.parseOptions(argc, argv) == FAIL){
        return FAIL;
    }

    unique_ptr<Config>pConfig(ap.GetConfig());
    MTEncoder encoder;
    int32_t res = encoder.Init(*(pConfig.get()));
    if(res == OK){
        encoder.Start(true);
    }else{
        return res;
    }

    return 0;
}
