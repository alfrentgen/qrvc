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

uint32_t MTDecoder::Init(istream* is, ostream* os, int32_t frameWidth, int32_t frameHeight, DecodeLib decLib, uint32_t framesPerThread, uint32_t nThreads){
    //calculate number of threads and input queue size
    if(nThreads == 0){
        m_nThreads = std::thread::hardware_concurrency();
        m_nThreads = (m_nThreads == 0) ? 2 : m_nThreads;
    }
    int32_t queueSize;
    if(framesPerThread == 0){
        framesPerThread = 8;
    }
    queueSize = framesPerThread * m_nThreads;

    m_inQ = new InputQueue(is, queueSize, frameWidth * frameHeight);
    m_outQ = new OutputQueue(os, queueSize, frameWidth * frameHeight);

    m_decLib = decLib;
    m_threads.clear();

    m_jobs.resize(m_nThreads);


    switch(m_decLib){
    case QUIRC:
        for(int i =0; i < m_nThreads; i++){
            m_jobs[i] = new DecodeQ(frameWidth, frameHeight, m_inQ, m_outQ);
        }
        break;
    case ZBAR:
    default:
        for(int i =0; i < m_nThreads; i++){
            m_jobs[i] = new Decode(frameWidth, frameHeight, m_inQ, m_outQ);
        }
        break;
    }

    return OK;
}

uint32_t MTDecoder::Start(bool join){

    m_threads.clear();
    try{
        for(int i = 0; i < m_nThreads; i++){
            //m_threads[i] = thread(&Decode::Do, m_jobs[i]);
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

uint32_t MTDecoder::Stop(){
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

    //system("pwd");
    //parse arguments
    ArgsParserDec ap = ArgsParserDec();
    if(ap.parseOptions(argc, argv) == FAIL){
        return FAIL;
    }

    map<string, string>& optionsMap = ap.getOptions();

    DecodeLib lib = ZBAR;
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
    LOG("Number of working threads is: %d\n", nThreads);

    key = string("-m");
    it = optionsMap.find(key);
    if(it != optionsMap.end()){
        if(it->second == string("quick")){
            lib = QUIRC;
        }
        else {
            lib = ZBAR;
        }
    }

    //create MTDecoder instance and Init
    MTDecoder decoder;

    decoder.Init(inputStream, outputStream, frameWidth, frameHeight, lib, framesPerThread, nThreads);
    decoder.Start(true);

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
