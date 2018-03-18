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

    ifstream* ifs = NULL;
    ofstream* ofs = NULL;
    istream* inputStream = &cin;
    ostream* outputStream = &cout;

    if(m_pKeyFileStream && m_pKeyFileStream->is_open()){
        m_pKeyFileStream->close();
    }
    m_pKeyFileStream = NULL;
    if(config.m_cipheringOn && !config.m_keyFileName.empty()){
        m_pKeyFileStream = new ifstream(config.m_keyFileName, ios_base::in | ios_base::binary);
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

    m_config = config;
    printDecCfg(m_config);

    int32_t queueSize = config.m_framesPerThread * config.m_nWorkingThreads;
    int32_t frameSize = config.m_frameWidth * config.m_frameHeight;
    if(m_pKeyFileStream){
        m_keyFrame.resize(frameSize);
        m_pKeyFileStream->read(m_keyFrame.data(), frameSize);
        int32_t bytesRead = m_pKeyFileStream->gcount();
        if(bytesRead < frameSize){
            LOG("Not enough bytes in key file: %d!\n", bytesRead);
            return FAIL;
        }else{
            LOG("Key frame bytes read: %d\n", bytesRead);
        }
    }else{
        m_keyFrame.resize(0);
    }
    vector<uint8_t>* pkeyFrame = config.m_cipheringOn ? &m_keyFrame : NULL;

    m_inQ = new InputQueue(inputStream, queueSize, frameSize);
    m_outQ = new OutputQueue(outputStream, queueSize, frameSize);

    m_threads.clear();
    m_jobs.resize(config.m_nWorkingThreads);

    for(int i =0; i < config.m_nWorkingThreads; i++){
        m_jobs[i] = new Decode(config, m_inQ, m_outQ);
        m_jobs[i]->SetCypheringParams(pkeyFrame);
    }

    return OK;
}

int32_t MTDecoder::Start(bool join){

    m_threads.clear();
    try{
        //LOG("Strating %d threads.\n", m_nThreads);
        for(int i = 0; i < m_config.m_nWorkingThreads; i++){
            m_threads.push_back(thread(&Decode::Do, m_jobs[i]));
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

int32_t MTDecoder::Stop(){
    for(int i = 0; i < m_config.m_nWorkingThreads; i++){
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
        LOG("\n");
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
