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


    //calculate number of threads and input queue size
    int32_t& nThreads = config.m_nWorkingThreads;
    if(nThreads == 0){
        m_nThreads = std::thread::hardware_concurrency();
        m_nThreads = (m_nThreads == 0) ? 2 : m_nThreads;
    }else{
        m_nThreads = nThreads;
    }

    int32_t queueSize;
    int32_t& framesPerThread = config.m_framesPerThread;
    if(framesPerThread == 0){
        framesPerThread = 8;
    }
    queueSize = framesPerThread * m_nThreads;

    int32_t& frameWidth = config.m_frameWidth;
    int32_t& frameHeight = config.m_frameHeight;
    if(frameWidth <= 0 || frameHeight <= 0){
        return FAIL;
    }
    m_inQ = new InputQueue(inputStream, queueSize, frameWidth * frameHeight);
    m_outQ = new OutputQueue(outputStream, queueSize, frameWidth * frameHeight);

    if(config.m_decMode < 0 || config.m_decMode > 2){
        return FAIL;
    }

    m_decMode = (DecodeMode)config.m_decMode;
    m_threads.clear();

    m_jobs.resize(m_nThreads);

    for(int i =0; i < m_nThreads; i++){
        m_jobs[i] = new Decode(frameWidth, frameHeight, m_inQ, m_outQ, m_decMode);
    }

    LOG("Number of working threads is: %d\n", m_nThreads);

    return OK;
}

int32_t MTDecoder::Init(istream* is, ostream* os, int32_t frameWidth, int32_t frameHeight, DecodeMode decMode, uint32_t framesPerThread, uint32_t nThreads){
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
        m_jobs[i] = new Decode(frameWidth, frameHeight, m_inQ, m_outQ, m_decMode);
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

    //system("pwd");
    //parse arguments
    ArgsParserDec ap = ArgsParserDec();
    if(ap.parseOptions(argc, argv) == FAIL){
        return FAIL;
    }

/*    map<string, string>& optionsMap = ap.getOptions();

    DecodeMode decMode = MIXED;
    uint32_t frameWidth = 0;
    uint32_t frameHeight = 0;
    bool frameCounter = 0;
    int32_t framesPerThread = 0;
    int32_t nThreads = 0;
    ifstream* ifs = NULL;
    ofstream* ofs = NULL;
    istream* inputStream = &cin;
    ostream* outputStream = &cout;

    //opening IS and OS
    string key = string("-i");
    map<string, string>::iterator it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "Input filename is not specified, reading from stdin.\n";
    }else{
        ifs = new ifstream(it->second, ios_base::in | ios_base::binary);
        inputStream = ifs;
        if (!ifs->is_open() || !ifs){
            cerr << "Failed to open input stream.";
            return FAIL;
        }
    }

    key = string("-o");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "Output filename is not specified, writing to stdout.\n";
    }else{
        ofs = new ofstream(it->second, ios_base::out | ios_base::binary);
        outputStream = ofs;
        if (!ofs->is_open() || !ofs){
            cerr << "Failed to open output stream.";
            return FAIL;
        }
    }

    //not used yet
    key = string("-c");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "Counter disabled.\n";
    }else{
        cerr << "Counter enabled.\n";
        frameCounter = 1;
    }

    key = string("-f");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "No frame size specified, using 800x600";
        frameWidth = 800;
        frameHeight = 600;
    }else{
        string sizeStr = it->second;
        regex exp = regex("\\d{1,4}");
        smatch result;
        uint32_t* widthHeight[2] = {&frameWidth, &frameHeight};

        for(int i = 0; regex_search(sizeStr, result, exp) && i < 2; ++i){
            string found = result[0];
            cerr << found << endl;
            sizeStr = result.suffix().str();
            *(widthHeight[i]) = stoi(found);
        }
    }

    key = string("-p");
    it = optionsMap.find(key);
    if(it != optionsMap.end()){
        framesPerThread = stoi(it->second);
    }

    key = string("-w");
    it = optionsMap.find(key);
    if(it != optionsMap.end()){
        nThreads = stoi(it->second);
    }

    decMode = MIXED;
    key = string("-m");
    it = optionsMap.find(key);
    if(it != optionsMap.end()){
        if(it->second == string("quick")){
            decMode = QUICK;
        }else if(it->second == string("slow")){
            decMode = SLOW;
        }else if(it->second == string("mixed")){
            decMode = MIXED;
        }
    }*/

    unique_ptr<Config>pConfig(ap.GetConfig());

    //create MTDecoder instance and Init
    MTDecoder decoder;

    //decoder.Init(inputStream, outputStream, frameWidth, frameHeight, decMode, framesPerThread, nThreads);
    int32_t res = decoder.Init(*(pConfig.get()));
    pConfig.reset();
    if(res == OK){
        decoder.Start(true);
    }

    /*if(ifs && inputStream != &cin){
        ifs->close();
        delete inputStream;
    }
    if(ofs && outputStream != &cout){
        ofs->close();
        delete outputStream;
    }*/
    return 0;
}
