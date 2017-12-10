#include <MTEncoder.h>
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
uint32_t MTDecoder::Init(istream* is, ostream* os, int32_t frameWidth, int32_t frameHeight, uint32_t chunkSize,
                         uint32_t framesPerThread, uint32_t nThreads){
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

    m_threads.clear();

    m_jobs.resize(m_nThreads);

    for(int i =0; i < m_nThreads; i++){
        m_jobs[i] = new Encode(frameWidth, frameHeight, m_inQ, m_outQ);
    }

    LOG("Number of working threads is: %d\n", m_nThreads);

    return OK;
}

uint32_t MTDecoder::Start(bool join){

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

    uint32_t frameWidth = 800;
    uint32_t frameHeight = 600;
    uint64_t chunkCounter = 0;
    int32_t frameRepeats = 1;
    int32_t tail = 0;
    istream* inputStream = &cin;
    ostream* outputStream = &cout;
    uint32_t chunkSize = 0;
    uint32_t qrScale = 1;
    QRecLevel eccLevel = QR_ECLEVEL_L;

    string key = string("-i");
    map<string, string>::iterator it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "No input filename was specified, reading from stdin.\n";
    }else{
        inputStream = new ifstream(it->second, ios_base::in | ios_base::binary);
        if (!inputStream->is_open() || !inputStream){
            cerr << "Failed to open input stream.";
            return FAIL;
        }
    }

    key = string("-o");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "No output filename was specified, writing to stdout.\n";
    }else{
        outputStream = new ofstream(it->second, ios_base::out | ios_base::binary);
        if (!outputStream->is_open() || !outputStream){
            cerr << "Failed to open output stream.";
            return FAIL;
        }
    }

    //inputStream->tie(outputStream);

    /*key = string("-c");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "Counter is disabled.\n";
    }else{
        cerr << "Counter is enabled.\n";
        frameCounter = 1;
    }*/

    key = string("-f");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "No frame size was specified, using 800x600.\n";
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

    key = string("-s");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "No qr scale was specified, using 1.\n";
        qrScale = 1;
    }else{
        qrScale = stoi(it->second);

    }

    key = string("-e");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "No ECC level was specified, using the lowest!\n";
        eccLevel = QR_ECLEVEL_L;
    }else{
        uint32_t l= stoi(it->second);
        switch(l){
        case QR_ECLEVEL_L:
            eccLevel = QR_ECLEVEL_L;
            break;
        case QR_ECLEVEL_M:
            eccLevel = QR_ECLEVEL_M;
            break;
        case QR_ECLEVEL_Q:
            eccLevel = QR_ECLEVEL_Q;
            break;
        case QR_ECLEVEL_H:
            eccLevel = QR_ECLEVEL_H;
            break;
        default:
            eccLevel = QR_ECLEVEL_L;
            break;
        }
    }

    key = string("-r");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "No number of frame repeats was specified, using 0.\n";
        frameRepeats = 1;
    }else{
        frameRepeats = stoi(it->second);
        if(frameRepeats <= 0){
            frameRepeats = 1;
        }else if(frameRepeats > 10){
            frameRepeats = 10;
        }
        cerr << "Number of frame repeats is " << frameRepeats << endl;
    }

    key = string("-t");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "No number of trailing frame was specified, using 0.\n";
        tail = 0;
    }else{
        tail = stoi(it->second);
        if(tail < 0){
            tail = 0;
        }else if(tail > 99){
            tail = 99;
        }
        cerr << "Number of trailing frames is " << tail << endl;
    }

    uint32_t version = 0;
    chunkSize = getChunkSize(frameWidth, frameHeight, eccLevel, qrScale, &version);
    if(!chunkSize || !version){
        cerr << "Frame size does not fit any possible qr code. Try smaller scale, ECC  level or bigger frame.\n";
        return 0;
    }else{
        cerr << "QR version: " << version << endl;
    }

    int32_t nBytesToRead = chunkSize - COUNTER_SIZE - HASHSUM_SIZE;
    cerr << "Chunk size: " << chunkSize << endl;
    cerr << "Bytes to read: " << nBytesToRead << endl;

    //create MTDecoder instance and Init
    MTEncoder encoder;

    encoder.Init(inputStream, outputStream, frameWidth, frameHeight, framesPerThread, nThreads, );
    encoder.Start(true);

    return 0;
}
