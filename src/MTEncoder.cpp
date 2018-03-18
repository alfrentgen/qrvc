#include <MTEncoder.h>
#include "utilities.h"
#include "help.h"

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

MTEncoder::MTEncoder():
m_pKeyFileStream(NULL)
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

    if(m_pKeyFileStream && m_pKeyFileStream->is_open()){
        m_pKeyFileStream->close();
    }
    m_pKeyFileStream = NULL;
    if(config.m_cipheringOn && !config.m_keyFileName.empty()){
        m_pKeyFileStream = new ofstream(config.m_keyFileName, ios_base::out | ios_base::binary);
        if(!m_pKeyFileStream->good()){
            m_pKeyFileStream->close();
            return FAIL;
        }
    }

    if(config.m_ifName.size() != 0){
        ifs = new ifstream(config.m_ifName, ios_base::in | ios_base::binary);
        if (!ifs || !ifs->is_open()){
            ifs = NULL;
            LOG("Failed to open input stream.");
            return FAIL;
        }
        inputStream = ifs;
    }

    if(config.m_ofName.size() != 0){
        ofs = new ofstream(config.m_ofName, ios_base::out | ios_base::binary);
        if (!ofs || !ofs->is_open()){
            ofs = NULL;
            LOG("Failed to open output stream.");
            return FAIL;
        }
        outputStream = ofs;
    }

    if(ValidateConfig(config) != OK){
        return FAIL;
    }

    //Init2
    uint32_t version = 0;
    int32_t chunkSize = getChunkSize(config.m_frameWidth - config.m_alignment, config.m_frameHeight - config.m_alignment,
                                        config.m_eccLevel, config.m_qrScale, &version);
    if(!chunkSize || version > 40 || version <= 0){
        LOG("Frame size does not fit any possible qr code. Try smaller scale, ECC  level, bigger frame or changing alignment.\n");
        return FAIL;
    }
    //LOG("QR version: %d\n", version);
    config.m_qrVersion = version;

    m_config = config;//accept config, as it is counted valid from now
    printEncCfg(m_config);

    int32_t nBytesToRead = chunkSize - COUNTER_SIZE - HASHSUM_SIZE;
    int32_t queueSize = config.m_framesPerThread * config.m_nWorkingThreads;
    m_inQ = new InputQueue(inputStream, queueSize, nBytesToRead);
    m_outQ = new OutputQueue(outputStream, queueSize, config.m_frameWidth * config.m_frameHeight);

    m_threads.clear();
    m_jobs.resize(config.m_nWorkingThreads);

    m_keyQR.resize(0);
    vector<uint8_t> * const pKeyFrame = config.m_cipheringOn ? &m_keyQR : NULL;

    for(int i =0; i < config.m_nWorkingThreads; i++){
        m_jobs[i] = new Encode(m_config, m_inQ, m_outQ);
        m_jobs[i]->SetCypheringParams(pKeyFrame, m_pKeyFileStream);
    }

    return OK;
}

int32_t MTEncoder::Start(bool join){

    m_threads.clear();
    try{
        //LOG("Strating %d threads.\n", m_nThreads);
        for(int i = 0; i < m_config.m_nWorkingThreads; i++){
        //LOG("Strating thread #%d.\n", i);
            m_threads.push_back(thread(&Encode::Do, m_jobs[i]));
        }

        if(join){
            for(int i = 0; i < m_config.m_nWorkingThreads; i++){
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
    for(int i = 0; i < m_config.m_nWorkingThreads; i++){
        m_jobs[i]->Stop();
    }
    for(int i = 0; i < m_threads.size(); i++){
        if(m_threads[i].joinable()){
            m_threads[i].join();
        }
    }

    m_threads.clear();
    m_keyQR.clear();

    if(m_pKeyFileStream && m_pKeyFileStream->is_open()){
        m_pKeyFileStream->flush();
        m_pKeyFileStream->close();
    }
    m_pKeyFileStream = NULL;

    //need to wait for threads to be stopped
    //and streams to be closed
    return 0;
}

int main(int argc, char** argv){

    if(argc <= 1){
        print_help(string("common"));
        print_help(string("encoder"));
        LOG("\n");
        exit(0);
    }

    ArgsParser ap = ArgsParser();
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

int32_t MTEncoder::ValidateConfig(Config& config){

    //frame
    LIMIT_VAR(config.m_alignment, 0, 64);//64 is the size of macroblock in x264
    LIMIT_VAR(config.m_frameHeight, 21, 1920);//21 is the smallest size of qr code
    LIMIT_VAR(config.m_frameWidth, 21, 1920);

    //qr
    LIMIT_VAR(config.m_qrScale, 1, 10);
    LIMIT_VAR(config.m_eccLevel, ECC_LEVEL_L, ECC_LEVEL_Q);
    /****
    config.m_qrVersion;//must be calculated in Init()
    ****/

    //frame sequence
    LIMIT_VAR(config.m_frameRepeats, 1, 9);
    LIMIT_VAR(config.m_nTrailingFrames, 0, 99);

    //system
    LIMIT_VAR(config.m_framesPerThread, 1, 99);
    /********************
    config.m_nWorkingThreads;//already detected in ArgsParser::GetConfig(), because it is common for decoder and encoder.
    //the next must be checked in Init()
    config.m_ofName;
    config.m_ifName;
    config.m_keyFileName;
    *********************/

    return OK;
}
