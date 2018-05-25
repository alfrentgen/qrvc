#include "steganography.h"

#define STEG_UNIT_SIZE 4

vector<uint8_t> spiralMatrix(int32_t qrWidth){
    //currentIdx
    vector<uint8_t> indeces(0);

    int32_t sideLength = qrWidth - 1;
    int32_t index = 0;

    while(sideLength > 0){

        for(int i = 0; i < 4; i++){
            int32_t direction = (i >= 2) ? -1 : 1;
            int32_t stride = i%2 ? sideLength : 1;
            int32_t diff = direction * stride;

            for(int j = 0; j < sideLength; j++){
                indeces.push_back(index + diff);
            }
        }
        sideLength -= 2;
    }
    return indeces;
}

vector<uint8_t> generateDefaultQRPath(int32_t qrWidth){
    spiralMatrix(qrWidth);
}

vector<uint8_t> generateQRPath(int32_t qrWidth, function<vector<uint8_t>(int32_t)>* customAlg){
    if(customAlg)
        return (*customAlg)(qrWidth);
    else
        return generateDefaultQRPath(qrWidth);
}

int32_t getStride(int32_t size){
    int32_t units = size/STEG_UNIT_SIZE - 1;

    int32_t borderSize = size%STEG_UNIT_SIZE;
    if(borderSize == 0){
        units -= 1;
    }
    return units;
}

vector<tuple<uint8_t, uint8_t>> generateDefaultFramePath_NoKey(int32_t frameWidth, int32_t frameHeight){
    vector<tuple<uint8_t, uint8_t>> path(0);
    //left and top borders should be at least 4 pels sized
    //right and bottom borders should be not zero
    //A steg unit is 4x4 pels
    //border pixels(units) must not be used
    int32_t strideX = getStride(frameWidth);
    int32_t strideY = getStride(frameHeight);

    return path;
}

vector<tuple<uint8_t, uint8_t>> generateDefaultFramePath_Key(int32_t frameWidth, int32_t frameHeight){
    vector<tuple<uint8_t, uint8_t>> path(0);
    //left and top borders should be at least 4 pels sized
    //right and bottom borders should be not zero
    //A steg unit is 4x4 pels
    //border pixels(units) must not be used
    int32_t strideX = getStride(frameWidth);
    int32_t strideY = getStride(frameHeight);


    //Just generating random  matrix

    return path;
}

//tuple<raw, column>
vector<tuple<uint8_t, uint8_t>> generateFramePath(int32_t frameWidth, int32_t frameHeight,
    function<vector<tuple<uint8_t, uint8_t>>(int32_t, int32_t)>* defaultAlg,
    function<vector<tuple<uint8_t, uint8_t>>(int32_t, int32_t)>* customAlg)
{
    if(customAlg)
        return (*customAlg)(frameWidth, frameHeight);
    else
        return (*defaultAlg)(frameWidth, frameHeight);
}
