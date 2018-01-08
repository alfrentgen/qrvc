#include "MTDecoder.h"
#include "utilities.h"


MTDecoder::MTDecoder()
{
    //ctor
}

MTDecoder::~MTDecoder()
{
    //dtor
    Stop();
}

int32_t MTDecoder::Init(Config& config){
    istream* inputStream = &cin;
    ostream* outputStream = &cout;

    ifstream* ifs = NULL;
    ofstream* ofs = NULL;

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

    if(config.m_decMode < 0 || config.m_decMode > 2){
        return FAIL;
    }

    Init(inputStream, outputStream, config.m_frameWidth, config.m_frameHeight,
        config.m_decMode, config.m_framesPerThread, config.m_nWorkingThreads, config.m_skipDupFrames);

    LOG("Number of working threads is: %d\n", m_nThreads);

    return OK;
}

int32_t MTDecoder::Init(istream* is, ostream* os, int32_t frameWidth, int32_t frameHeight,
                        DecodeMode decMode, uint32_t framesPerThread, uint32_t nThreads, bool skipDup){
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

    m_inQ = new InputQueue(is, queueSize, frameWidth * frameHeight);
    m_outQ = new OutputQueue(os, queueSize, frameWidth * frameHeight);

    m_decMode = decMode;
    m_threads.clear();

    m_jobs.resize(m_nThreads);

    for(int i =0; i < m_nThreads; i++){
        m_jobs[i] = new Decode(frameWidth, frameHeight, m_inQ, m_outQ, m_decMode, skipDup);
    }

    LOG("Number of working threads is: %d\n", m_nThreads);

    return OK;
}

int32_t MTDecoder::Start(bool join){

    m_threads.clear();
    try{
        LOG("Strating %d threads.\n", m_nThreads);
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

    //need to wait for threads to be stopped
    //and streams to be closed
    return 0;
}

int main(int argc, char** argv){
    ArgsParserDec ap = ArgsParserDec();
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
