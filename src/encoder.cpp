#include "encoder.h"

extern "C" {
    #include <qrencode.h>
    #include <qrspec.h>
    #include <qrinput.h>
}

#define STR(s) #s

#define MAX_BIN_CHUNKSIZE 2953
#define NOT_MQR 0

using namespace std;

uint32_t CalcHashsum(uint8_t* pBuffer, uint32_t bufSize){
    uint32_t hash = bufSize;
    for(uint32_t i = 0; i < bufSize; i++){
        hash = hash + (hash << 2);
        hash += pBuffer[i];
    }
    return hash;
}

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

int main (int argc, char **argv)
{
    ArgsParserDec* ap = new ArgsParserDec();
    ap->parseOptions(argc, argv);

    map<string, string>& optionsMap = ap->getOptions();

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
    }

    key = string("-o");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "No output filename was specified, writing to stdout.\n";
    }else{
        outputStream = new ofstream(it->second, ios_base::out | ios_base::binary);
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
    }else{
        qrScale = stoi(it->second);

    }

    key = string("-e");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "No ECC level was specified, using the lowest!\n";
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

    int32_t nBytesToRead = chunkSize - 8 - 4;
    vector<uint8_t> rawFrame = vector<uint8_t>(frameWidth * frameHeight, 255);//fill with white pells
    vector<uint8_t> chunk = vector<uint8_t>(chunkSize, 0);

    cerr << "Chunk size: " << chunkSize << endl;
    cerr << "Bytes to read: " << nBytesToRead << endl;
    //cerr << "frame width: " << frameWidth << endl;
    //cerr << "frame height: " << frameHeight << endl;
    // create a reader
    // read from binary file:
    do{
        uint8_t* pInData = chunk.data();
        inputStream->read((char*)pInData + 8, nBytesToRead);
        int32_t bytesRead = inputStream->gcount();
        //do nothing if read nothing
        if(bytesRead == 0){
            continue;
        }
        //put frame ID in data to encode litle endian lesser byte first
        for(int i = 0; i < 8; i++){
            int32_t shift = 8 * i;
            pInData[i] = (uint8_t)(chunkCounter >> shift & (uint64_t)0xff);
        }

        //put hashsum in data to encode litle endian lesser byte first
        uint32_t hashsum = CalcHashsum(pInData, bytesRead + 8);
        pInData += bytesRead + 8;
        for(int i = 0; i < 4; i++){
            int32_t shift = 8 * i;
            pInData[i] = (uint8_t)((hashsum >> shift) & (uint32_t)0xff);
        }

        //cerr << "bytesRead = " << bytesRead << endl;
        if(bytesRead < nBytesToRead){
            cerr << "Incomplete frame have been received! Proceeding with input chunk of:" << inputStream->gcount() << " bytes.\n";
        }

        QRcode* pQR = QRcode_encodeData(bytesRead + 8 + 4, (unsigned char*)chunk.data(), version, eccLevel);

        //put scaled code in the center of rawFrame
        //positioning
        uint8_t* pQRData = pQR->data;
        int32_t qrWidth = pQR->width;
        uint32_t xOffset = (frameWidth - qrScale * qrWidth)/2;
        uint32_t yOffset = (frameHeight - qrScale * qrWidth)/2;
        vector<uint8_t>::iterator frameIt = rawFrame.begin() + yOffset * frameWidth;

        for(int32_t y = 0; y < qrWidth; y++){
            for(int32_t x = 0; x < qrWidth; x++){
                uint8_t val = (0x01 & pQRData[x + y * qrWidth]) ? 0 : 255;
                fill_n(frameIt + xOffset + x * qrScale, qrScale, val);
            }
            frameIt += frameWidth;
            for(uint32_t cnt = 1; cnt < qrScale; ++cnt){
                copy_n(frameIt-frameWidth, frameWidth, frameIt);
                frameIt += frameWidth;
            }
        }
        //writring bytes to output stream

        for(int r = 0; r < frameRepeats; r++){
            outputStream->write((char*)rawFrame.data(), rawFrame.size());
        }

        outputStream->flush();
        //cerr << chunkCounter << endl;
        chunkCounter++;
        //delete pQR;

    }while(!inputStream->eof());

    while(tail--){
        outputStream->write((char*)rawFrame.data(), rawFrame.size());
    }
    outputStream->flush();

    delete ap;
    return(0);
}
