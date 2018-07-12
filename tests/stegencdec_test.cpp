#include "steganography.h"
#include "Config.h"

#include <quirc.h>
#include <quirc_internal.h>
extern "C" {
    #include <qrencode.h>
    #include <qrspec.h>
    #include <qrinput.h>
}

#define WIDTH 1280
#define HEIGHT 720
#define QR_WIDTH 177
#define QR_SCALE 4
#define THRESHOLD 8
#define KEY_PRESENTED true//false

void dump(char* fName, uint8_t* signal, uint32_t size) {
	FILE* dumpFile = fopen(fName, "wb");
	fwrite((void*)signal, sizeof(uint8_t), size, dumpFile);
	fclose(dumpFile);
	return;
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

    int32_t size = QRspec_getDataLength(version, eccLevel);
    if(version > 9){
        size -= (2 + 1);
    }else{
        size -= (1 + 1);
    }
    *retVersion = version;
    return size;
}

int32_t main(){
    //setting up
    QRecLevel eccLevel = ECC_LEVEL_Q;
    uint32_t version = 0;
    uint32_t chunkSize = getChunkSize(WIDTH, HEIGHT, eccLevel, QR_SCALE, &version);
    int32_t qrWidth = QRspec_getWidth(version);
    //generating chunk
    vector<uint8_t> chunk(chunkSize);
    vector<uint8_t> decChunk(chunkSize);
    srand (time(NULL));
    for(int i = 0; i < chunkSize; i++){
        chunk[i] = rand() % 256;
    }
    //encoding
    QRcode* pQR = QRcode_encodeData(chunk.size(), (unsigned char*)chunk.data(), version, eccLevel);
    uint8_t* pQRData = pQR->data;
    LOG("qrWidth = %d, pQR->width = %d!\n", qrWidth, pQR->width);
    if(qrWidth != pQR->width){
        return FAIL;
    }
    //decoding
    quirc_code quircCode;
    struct quirc_data data;
    memset(&quircCode, 0, sizeof(quircCode));
    memset(&data, 0, sizeof(data));
    quircCode.size = qrWidth;
    for(int i = 0; i < qrWidth*qrWidth; i++){
        if(pQRData[i] != 0){
            quircCode.cell_bitmap[i >> 3] & (1 << (i & 7));//where i = (y * size) + x
        }
    }
    quirc_decode_error_t err = quirc_decode(&quircCode, &data);
    if (err){
        LOG("Quick decode failed: %s\n", quirc_strerror(err));
        return FAIL;
    }else{
        decChunk.assign(data.payload, data.payload + data.payload_len);
    }


    /*StegModule module;
    vector<uint8_t> frame(WIDTH * HEIGHT, 127);
    vector<uint8_t> qrCode(qrWidth * qrWidth, 0);

    for(int i = 0; i < qrCode.size(); i++){
        qrCode[i] = (i%2) * 255;
    }
    //std::random_shuffle(qrCode.begin(),qrCode.end());

    cout << "init\n";
    int32_t res = module.Init(WIDTH, HEIGHT, QR_WIDTH, THRESHOLD, KEY_PRESENTED);
    int i = 0;

    cout << "hide\n";
    module.Hide(frame.data(), qrCode.data());

    string fName("stegaDump.yuv");
    cout << fName << endl;
    dump(const_cast<char*>(fName.c_str()), frame.data(), frame.size());*/

    return 0;
}
