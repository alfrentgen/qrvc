#ifndef STEGANOGRAPHY_H
#define STEGANOGRAPHY_H
#include <pch.h>

using namespace std;
class StegaModule{
public:
    int32_t Hide(uint8_t* frame, int32_t width, int32_t height, uint8_t* qrCode, int32_t qrWidth);
    int32_t GeneratePaths(int32_t frameWidth, int32_t frameHight, int32_t qrWidth);
    vector<uint8_t> m_QRPath;
    vector<uint8_t> m_framePath;
    bool m_keyFlag;
}

vector<uint8_t> generateQRPath(int32_t qrWidth, function<vector<uint8_t>(int32_t)>* customAlg);

vector<uint8_t> generateFramePath(int32_t frameWidth, int32_t frameHeight,
    function<uint8_t>(int32_t, int32_t)>* defaultAlg,
    function<uint8_t>(int32_t, int32_t)>* customAlg
    );

vector<uint8_t> generateDefaultFramePath(int32_t frameWidth, int32_t frameHeight);
vector<uint8_t> generateDefaultFrameKeyPath(int32_t frameWidth, int32_t frameHeight);

#endif
