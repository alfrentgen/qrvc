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
uint32_t MTEncoder::Init(istream* is, ostream* os, int32_t frameWidth, int32_t frameHeight, QRecLevel eccLevel, int32_t qrScale,
                         uint32_t framesPerThread, uint32_t nThreads){
    //check if frame size fits QR code size
    uint32_t version = 0;
    int32_t chunkSize = getChunkSize(frameWidth, frameHeight, eccLevel, qrScale, &version);
    if(!chunkSize || !version){
        cerr << "Frame size does not fit any possible qr code. Try smaller scale, ECC  level or bigger frame.\n";
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

    for(int i =0; i < m_nThreads; i++){
        m_jobs[i] = new Encode(frameWidth, frameHeight, m_inQ, m_outQ);
    }

    LOG("Number of working threads is: %d\n", m_nThreads);

    return OK;
}

uint32_t MTEncoder::Start(bool join){

    m_threads.clear();
    try{
        LOG("Strating %d threads.\n", m_nThreads);
        for(int i = 0; i < m_nThreads; i++){
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

uint32_t MTEncoder::Stop(){
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
    int32_t framesPerThread = 0;
    int32_t nThreads = 0;
    int32_t tail = 0;
    ifstream* ifs = NULL;
    ofstream* ofs = NULL;
    istream* inputStream = &cin;
    ostream* outputStream = &cout;
    uint32_t chunkSize = 0;
    uint32_t qrScale = 1;
    QRecLevel eccLevel = QR_ECLEVEL_L;

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

    //create MTDecoder instance and Init
    MTEncoder encoder;

    encoder.Init(inputStream, outputStream, frameWidth, frameHeight, eccLevel, qrScale, framesPerThread, nThreads);
    encoder.Start(true);

    return 0;
}
