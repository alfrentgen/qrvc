#ifndef STEGANOGRAPHY_H
#define STEGANOGRAPHY_H
#include <pch.h>

int32_t Hide(uint8_t* frame, int32_t width, int32_t height, uint8_t* qrCode, int32_t qrwWidth);
int32_t nextQRPointIdx(int32_t currentIdx, int32_t qrWidth);

#endif
