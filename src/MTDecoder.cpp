#include "MTDecoder.h"
#include "utilities.h"
#include "help.h"
#include "Config.h"

MTDecoder::MTDecoder():
m_pKeyFileStream(NULL)
{
    //ctor
}

MTDecoder::~MTDecoder()
{
    //dtor
    Stop();
}

int32_t MTDecoder::ValidateConfig(Config& config){
    //frame
    LIMIT_VAR(config.m_frameHeight, 21, 1920);//21 is the smallest size of qr code
    LIMIT_VAR(config.m_frameWidth, 21, 1920);

    //stream
    LIMIT_VAR(config.m_decMode, QUICK, SLOW);
    //config.m_skipDupFrames;


    //system
    LIMIT_VAR(config.m_framesPerThread, 1, 99);
    /********************
    config.m_nWorkingThreads;//already detected in ArgsParser::GetConfig()
    because it is common for decoder and encoder.
    //the next must be checked in Init()
    config.m_ofName;
    config.m_ifName;
    config.m_keyFileName;
    *********************/

    return OK;
}

int32_t MTDecoder::Init(Config& config){
    istream* inputStream = &cin;
    ostream* outputStream = &cout;

    ifstream* ifs = NULL;
    ofstream* ofs = NULL;

    if(config.m_ifName.size() != 0){
        ifs = new ifstream(config.m_ifName, ios_base::in | ios_base::binary);
        if (!ifs || !ifs->is_open()){
            ifs = NULL;
            cerr << "Failed to open input stream.";
            return FAIL;
        }
        inputStream = ifs;
    }

    if(config.m_ofName.size() != 0){
        ofs = new ofstream(config.m_ofName, ios_base::out | ios_base::binary);
        if (!ofs || !ofs->is_open()){
            ofs = NULL;
            cerr << "Failed to open output stream.";
            return FAIL;
        }
        outputStream = ofs;
    }

    if(config.m_decMode < 0 || config.m_decMode > 2){
        return FAIL;
    }

    if(m_pKeyFileStream && m_pKeyFileStream->is_open()){
        m_pKeyFileStream->close();
    }
    m_pKeyFileStream = NULL;

    m_cypherOn = config.m_cipheringOn;
    if(m_cypherOn && !config.m_keyFileName.empty()){
        m_pKeyFileStream = new ifstream(config.m_keyFileName, ios_base::in | ios_base::binary);
        if(!m_pKeyFileStream->good()){
            m_pKeyFileStream->close();
            return FAIL;
        }
    }

    if(ValidateConfig(config) != OK){
        return FAIL;
    }

    Init(inputStream, outputStream, config.m_frameWidth, config.m_frameHeight,
        config.m_decMode, config.m_framesPerThread, config.m_nWorkingThreads, config.m_skipDupFrames);

    //LOG("Number of working threads is: %d\n", m_nThreads);

    return OK;
}

int32_t MTDecoder::Init(istream* is, ostream* os, int32_t frameWidth, int32_t frameHeight,
                        DecodeMode decMode, uint32_t framesPerThread, uint32_t nThreads, bool skipDup){

    int32_t queueSize;
    if(framesPerThread == 0){
        framesPerThread = 8;
    }
    m_nThreads = nThreads;
    queueSize = framesPerThread * m_nThreads;

    if(m_cypherOn){
        m_keyFrame.resize(0);
    }
    if(m_pKeyFileStream){
        m_keyFrame.resize(frameWidth * frameHeight);
        m_pKeyFileStream->read(m_keyFrame.data(), frameWidth * frameHeight);
        int32_t bytesRead = m_pKeyFileStream->gcount();
        if(bytesRead < frameWidth * frameHeight){
            LOG("Not enough bytes in key file: %d!\n", bytesRead);
            return FAIL;
        }else{
            LOG("Key frame bytes read: %d\n", bytesRead);
        }
    }
    vector<uint8_t>* pkeyFrame = m_cypherOn ? &m_keyFrame : NULL;

    m_inQ = new InputQueue(is, queueSize, frameWidth * frameHeight);
    m_outQ = new OutputQueue(os, queueSize, frameWidth * frameHeight);

    m_decMode = decMode;
    m_threads.clear();

    m_jobs.resize(m_nThreads);

    for(int i =0; i < m_nThreads; i++){
        m_jobs[i] = new Decode(frameWidth, frameHeight, m_inQ, m_outQ, m_decMode, skipDup);
        m_jobs[i]->SetCypheringParams(pkeyFrame);
    }

    return OK;
}

int32_t MTDecoder::Start(bool join){

    m_threads.clear();
    try{
        //LOG("Strating %d threads.\n", m_nThreads);
        for(int i = 0; i < m_nThreads; i++){
            m_threads.push_back(thread(&Decode::Do, m_jobs[i]));
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

int32_t MTDecoder::Stop(){
    for(int i = 0; i < m_nThreads; i++){
        m_jobs[i]->Stop();
    }
    for(uint32_t i = 0; i < m_threads.size(); i++){
        if(m_threads[i].joinable()){
            m_threads[i].join();
        }
    }

    m_threads.clear();
    m_keyFrame.clear();
    if(m_pKeyFileStream && m_pKeyFileStream->is_open()){
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
        print_help(string("decoder"));
        exit(0);
    }

    ArgsParser ap = ArgsParser();
    if(ap.parseOptions(argc, argv) == FAIL){
        return FAIL;
    }

    MTDecoder decoder;
    unique_ptr<Config>pConfig(ap.GetConfig());
    int32_t res = decoder.Init(*(pConfig.get()));
    pConfig.reset();
    if(res == OK){
        decoder.Start(true);
    }else{
        return res;
    }
    return 0;
}
