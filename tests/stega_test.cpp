#include "steganography.h"

#define WIDTH 1280
#define HEIGHT 720
#define QR_WIDTH 177
#define THRESHOLD 8
#define KEY_PRESENTED true//false

void dump(char* fName, uint8_t* signal, uint32_t size) {
	FILE* dumpFile = fopen(fName, "wb");
	fwrite((void*)signal, sizeof(uint8_t), size, dumpFile);
	fclose(dumpFile);
	return;
}

int32_t main(){
    StegModule module;
    vector<uint8_t> frame(WIDTH * HEIGHT, 127);
    vector<uint8_t> qrCode(QR_WIDTH * QR_WIDTH, 0);
    for(int i = 0; i < qrCode.size(); i++){
        qrCode[i] = (i%2) * 255;
    }
    //std::random_shuffle(qrCode.begin(),qrCode.end());

    cout << "init\n";
    int32_t res = module.Init(WIDTH, HEIGHT, THRESHOLD, !KEY_PRESENTED);
    int i = 0;
    /*for(int32_t idx : module.m_qrPath){
        i++;
        LOG("%d,", idx);
        if(!(i%QR_WIDTH)){
            LOG("\n");
        }
    }*/
    /*if(res == FAIL){
        LOG("%d\n", module.m_qrPath.size());
        LOG("%d\n", module.m_framePath.size());
        LOG("Init FAILED\n");
        return -1;
    }*/
    cout << "hide\n";
    module.Hide(frame.data(), qrCode.data());
    /*for(int32_t idx : module.m_qrPath){
        i++;
        LOG("qrCode[%d] = %d,\n", idx, qrCode[idx]);
        //LOG("%d,", idx);
        if(!(i%QR_WIDTH))
            LOG("\n");
    }*/
    string fName("stegaDump.yuv");
    cout << fName << endl;
    dump(const_cast<char*>(fName.c_str()), frame.data(), frame.size());

    return 0;
}
