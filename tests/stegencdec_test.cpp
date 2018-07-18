#include "steganography.h"
#include "Config.h"

#include <quirc.h>
#include <quirc_internal.h>
extern "C" {
    #include <qrencode.h>
    #include <qrspec.h>
    #include <qrinput.h>
}

#define WIDTH 1280//640//1280
#define HEIGHT 720//480//720
#define QR_WIDTH 177
#define QR_SCALE 4
#define THRESHOLD 8

int32_t frame_sizes[5][2]={
{100, 100}, {320, 240}, {640, 480}, {800, 600},  {1280, 720}
};

void dump(char* fName, uint8_t* signal, uint32_t size) {
	FILE* dumpFile = fopen(fName, "wb");
	fwrite((void*)signal, sizeof(uint8_t), size, dumpFile);
	fclose(dumpFile);
	return;
}

void fillDot(uint8_t* pDot, int32_t stride, int32_t scale, uint8_t color){
    for(int y = 0; y < scale; y++){
        for(int x = 0; x < scale; x++){
            pDot[y * stride + x] = color;
        }
    }
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

int32_t test(int32_t width, int32_t height);

int32_t main(){
    for(int i = 0; i < 5; i++){
        LOG("\nStegEncDec test# %d\n", i);
        int32_t res = test(frame_sizes[i][0], frame_sizes[i][1]);
        CHECK_FAIL(res);
    }
    return 0;
}

int32_t test(int32_t width, int32_t height){
    //setting up
    StegModule module;
    int32_t res = module.Init(width, height, THRESHOLD);
    if(res){
        LOG("Init failed!\n");
        return FAIL;
    }
    int32_t qrWidth = module.GetQRWidth();
    QRecLevel eccLevel = ECC_LEVEL_L;
    int32_t version = 0;
    uint32_t chunkSize = getChunkSize(qrWidth, qrWidth, eccLevel, 1, &version);
    LOG("Chunk size = %d\n", chunkSize);
    if(!chunkSize || !version){
        return FAIL;
    }
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

    //for manual check output code image generation
    /*int8_t bgColor = 255;
    int8_t drawColor = 0;
    vector<uint8_t> qrFrame(WIDTH * HEIGHT, bgColor);
    uint32_t offsetX = QR_SCALE;
    uint32_t offsetY = QR_SCALE;
    uint8_t* pRaw = qrFrame.data() + offsetY * WIDTH;
    for(int y = 0; y < qrWidth; y++){
        uint8_t* pDot = pRaw + offsetX;
        for(int x = 0; x < qrWidth; x++){
            uint8_t val = (0x01 & pQRData[x + y * qrWidth]) ? drawColor : bgColor;
            fillDot(pDot + x*QR_SCALE, WIDTH, QR_SCALE, val);
        }
        pRaw += QR_SCALE * WIDTH;
    }
    dump("EncodedFrame.yuv", qrFrame.data(), qrFrame.size());*/

    //decoding
    quirc_code quircCode;
    struct quirc_data data;
    memset(&quircCode, 0, sizeof(quircCode));
    memset(&data, 0, sizeof(data));
    quircCode.size = qrWidth;
    for(int i = 0; i < qrWidth*qrWidth; i++){
        if((0x01 & pQRData[i])){
        //if(pQRData[i] != 0){
            quircCode.cell_bitmap[i >> 3] |= (1 << (i & 7));
        }
    }
    quirc_decode_error_t err = quirc_decode(&quircCode, &data);
    if (err){
        LOG("Dec1: Quick decode failed: %s\n", quirc_strerror(err));
        return FAIL;
    }else{
        decChunk.assign(data.payload, data.payload + data.payload_len);
        LOG("Dec1: Quick decode: %s\n", quirc_strerror(err));
    }

    if(decChunk.size() != chunk.size()){
        printf("Dec1: Chunks sizes are not equal!\n");
        return -1;
    }
    if(memcmp(decChunk.data(), chunk.data(), decChunk.size()) != 0){
        printf("Dec1: Chunks data is not equal!\n");
        return -1;
    }else{
        printf("Dec1: Chunks data is equal!\n");
    }

    //steganography test
    LOG("\nTesting steganography:\n");
    vector<uint8_t> frame(width * height, 127);
    vector<uint8_t> qrCode(qrWidth * qrWidth, 255);

    for(int i = 0; i < qrCode.size(); i++){
        if((0x01 & pQRData[i])){
            qrCode[i] = 0;
        }
    }
    dump("CodeBeforeHide.yuv", qrCode.data(), qrCode.size());

    cout << "init\n";

    cout << "hide\n";
    module.Hide(frame.data(), qrCode.data());

    string fName("stegaDump.yuv");
    cout << fName << endl;
    dump(const_cast<char*>(fName.c_str()), frame.data(), frame.size());

    //revealing the code
    vector<uint8_t> qrCodeRev(qrWidth * qrWidth, 255);
    LOG("qrCodeRev.size() = %d\n", qrCodeRev.size());
    LOG("frame.size() = %d\n", frame.size());
    LOG("revealing...\n");
    module.Reveal(frame.data(), qrCodeRev.data());
    dump("CodeRevealed.yuv", qrCodeRev.data(), qrCode.size());
    LOG("revealed!\n");
    if(qrCode.size() != qrCodeRev.size()){
        printf("Codes sizes are not equal!\n");
        return -1;
    }
    if(memcmp(qrCode.data(), qrCodeRev.data(), qrCode.size()) != 0){
        printf("Reveal: Codes are not equal!\n");
        return -1;
    }else{
        printf("Reveal: Codes are equal!\n");
    }

    //decode
    decChunk.resize(0);
    memset(&quircCode, 0, sizeof(quircCode));
    memset(&data, 0, sizeof(data));
    pQRData = qrCodeRev.data();
    quircCode.size = qrWidth;
    for(int i = 0; i < qrWidth*qrWidth; i++){
        if(pQRData[i] == 0){
            quircCode.cell_bitmap[i >> 3] |= (1 << (i & 7));
        }
    }
    err = quirc_decode(&quircCode, &data);
    if (err){
        LOG("Quick decode failed: %s\n", quirc_strerror(err));
        return FAIL;
    }else{
        decChunk.assign(data.payload, data.payload + data.payload_len);
    }

    if(decChunk.size() != chunk.size()){
        printf("Dec2: Chunks sizes are not equal!\n");
        return -1;
    }
    if(memcmp(decChunk.data(), chunk.data(), decChunk.size() != 0)){
        printf("Dec2: Chunks data is not equal!\n");
        return -1;
    }else{
        printf("Dec2: Chunks data is equal!\n");
    }

    printf("Successfully decoded!\n");
    return 0;
}
