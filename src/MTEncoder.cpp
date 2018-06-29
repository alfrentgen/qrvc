#include <MTEncoder.h>
#include "utilities.h"
#include "help.h"

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

    int32_t size = QRspec_getDataLength(version, eccLevel);
    if(version > 9){
        size -= (2 + 1);
    }else{
        size -= (1 + 1);
    }
    *retVersion = version;
    return size;
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
    config.m_cipheringOn = config.m_stegModeOn ? false : config.m_cipheringOn;
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
        LOG("version: %d, chunkSize: %d\n", version, chunkSize);
        LOG("Frame size does not fit any possible qr code. Try smaller scale, ECC  level, bigger frame or changing alignment.\n");
        return FAIL;
    }

    config.m_qrVersion = version;
    int32_t nBytesToRead = chunkSize - COUNTER_SIZE - HASHSUM_SIZE;
    if(nBytesToRead <= 0){
        LOG("In the codec ECC levels 2 and 3 are deprecated for QR code version %d.\n", version);
        return FAIL;
    }

    //configurign steganography module
    StegModule* pStegModule = nullptr;
    if(config.m_stegModeOn){
        if(inputStream == &cin){//can't treat cin as main data input, due to using it for steg data
            LOG("In steg mode standard input stream can only be used for hiding video input.\n");
            return FAIL;
        }
        if(config.m_stegThreshold == 0){
            config.m_stegThreshold = 8;
        }
        config.m_qrScale = 4;
        int32_t qrWidth = QRspec_getWidth(version);
        int32_t res = m_stegModule.Init(config.m_frameWidth, config.m_frameHeight, qrWidth, config.m_stegThreshold, RANDOM_PATH);
        if(res == FAIL){
            return FAIL;
        }
        pStegModule = &m_stegModule;
        //if file exists and can be read, try to read frame path from it and give it to steg module
        if(config.m_keyFileName.size()){
            ifstream ifs(config.m_keyFileName, ios_base::in | ios_base::binary);
            vector<uint8_t> framePath(0);
            if(ifs.good()){
                while (ifs.good()) {
                    framePath.push_back((uint8_t)ifs.get());
                }
                uint32_t minFramePathLength = 2 * qrWidth * qrWidth;
                if(m_stegModule.SetCustomFramePath(framePath.data(), framePath.size()) != OK){
                    LOG("Not enough length of frame path: %d, should be %d!\n", framePath.size(), minFramePathLength);
                    return FAIL;
                }
            }else{
                LOG("Cannot read steganography key file!\n");
                return FAIL;
            }
        }else{
            LOG("Using generated steganography key file.\n");
        }
        m_stegModule.SetUnitPattern(config.m_unitPattern);
    }

    m_config = config;//accept config, as it is counted valid from now
    printEncCfg(m_config);

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
        m_jobs[i]->SetStegParams(pStegModule);
    }

    return OK;
}

int32_t MTEncoder::Start(bool join){

    m_threads.clear();
    try{
        if(m_config.m_stegModeOn && m_config.m_keyFileName.size() == 0){//write steg key file
            string keyFileName = m_config.m_ifName + ".stg";
            ofstream stegKeyOS(keyFileName, ios_base::out | ios_base::binary);
            if(stegKeyOS.bad()){
                LOG("Cannot write steganography key file \"%s\"! Terminate!\n", keyFileName.c_str());
                return FAIL;
            }
            vector<uint8_t> framePath8bit(0);
            for(int32_t i = 0; i < m_stegModule.m_framePath.size(); i++){
                framePath8bit.push_back((uint8_t)m_stegModule.m_framePath[i]);
            }
            stegKeyOS.write(framePath8bit.data(), framePath8bit.size());
        }

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
    for(int i = 0; i < m_jobs.size(); i++){
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
