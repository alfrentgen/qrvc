#ifndef STEGANOGRAPHY_H
#define STEGANOGRAPHY_H
#include <pch.h>

using namespace std;

int32_t Hide(uint8_t* frame, int32_t width, int32_t height, uint8_t* qrCode, int32_t qrwWidth);

vector<uint8_t> generateQRPath(int32_t qrWidth, function<vector<uint8_t>(int32_t)>* customAlg);

vector<tuple<uint8_t, uint8_t>> generateFramePath(int32_t frameWidth, int32_t frameHeight,
    function<vector<tuple<uint8_t, uint8_t>>(int32_t, int32_t)>* defaultAlg,
    function<vector<tuple<uint8_t, uint8_t>>(int32_t, int32_t)>* customAlg
    );

vector<tuple<uint8_t, uint8_t>> generateDefaultFramePath(int32_t frameWidth, int32_t frameHeight);
vector<tuple<uint8_t, uint8_t>> generateDefaultFrameKeyPath(int32_t frameWidth, int32_t frameHeight);

#endif
