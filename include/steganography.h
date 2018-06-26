#ifndef STEGANOGRAPHY_H
#define STEGANOGRAPHY_H
#include <pch.h>

using namespace std;

#define RANDOM_PATH true

typedef struct StegUnit{
    bool bit;
    uint8_t* pUnit;
    int32_t threshold;
    vector<int32_t> neighInds;
    vector<int32_t> coreInds;
    vector<uint8_t*> neighPels;
    vector<uint8_t*> corePels;
    StegUnit():
        bit(false), pUnit(nullptr), neighInds(0), coreInds(0){}
} StegUnit_t;

class StegModule{
public:
    int32_t Hide(uint8_t* frame, uint8_t* qrCode);
    int32_t Init(int32_t frameWidth, int32_t frameHeight, int32_t qrWidth, int32_t threshold, bool keyFlag);
    vector<int32_t> m_qrPath;
    vector<int32_t> m_framePath;
    bool m_keyFlag;
    int32_t m_frameWidth;
    int32_t m_frameHeight;
    int32_t m_qrWidth;
    int32_t m_threshold;
};

vector<int32_t> generateQRPath(int32_t qrWidth, function<vector<int32_t>(int32_t)>* customAlg);

vector<int32_t> generateFramePath(int32_t frameWidth, int32_t frameHeight, bool keyFlag,
    function<vector<int32_t>(int32_t, int32_t, bool)>* defaultAlg,
    function<vector<int32_t>(int32_t, int32_t, bool)>* customAlg);

int32_t writeFramePath(vector<uint8_t> path, char* fileName);

vector<uint8_t> readFramePath(char* fileName);

//vector<uint8_t> generateDefaultFramePath(int32_t frameWidth, int32_t frameHeight);
//vector<uint8_t> generateDefaultFrameKeyPath(int32_t frameWidth, int32_t frameHeight);

#endif
