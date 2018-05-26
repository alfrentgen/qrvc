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

int32_t getUnitsCount(int32_t size){
    int32_t units = size/STEG_UNIT_SIZE - 1;

    int32_t borderSize = size%STEG_UNIT_SIZE;
    if(borderSize == 0){
        units -= 1;
    }
    return units;
}

vector<uint8_t> generateDefaultFramePath_NoKey(int32_t frameWidth, int32_t frameHeight){
    vector<uint8_t> path(0);
    //left and top borders should be at least 4 pels sized
    //right and bottom borders should be not zero
    //A steg unit is 4x4 pels
    //border pixels(units) must not be used
    int32_t strideX = getUnitsCount(frameWidth);
    int32_t strideY = getUnitsCount(frameHeight);

    return path;
}

vector<uint8_t> generateDefaultFramePath_Key(int32_t frameWidth, int32_t frameHeight){
    vector<uint8_t> path(0);
    //left and top borders should be at least 4 pels sized
    //right and bottom borders should be not zero
    //A steg unit is 4x4 pels
    //border pixels(units) must not be used
    int32_t unitsX = getUnitsCount(frameWidth);
    int32_t unitsY = getUnitsCount(frameHeight);
    //int32_t nUnits = strideX * strideY;
    vector<uint8_t> xIndeces(unitsX);
    vector<uint8_t> yIndeces(unitsY);

    for(int i = 1; i <= unitsX; i++){
        xIndeces[i] = i;
    }

    for(int i = 1; i <= unitsY; i++){
        yIndeces[i] = i;
    }

    std::random_shuffle(xIndeces.begin(), xIndeces.end());
    std::random_shuffle(yIndeces.begin(), yIndeces.end());

    int32_t nTaps = unitsX < unitsY ? unitsX : unitsY;
    path.resize(2 * nTaps);
    for(int i = 0; i < nTaps; i++){
        path[2 * i] = xIndeces[i];
        path[2 * i + 1] = yIndeces[i];
    }
    //Just generating random  matrix

    return path;
}

//tuple<raw, column>
vector<uint8_t> generateFramePath(int32_t frameWidth, int32_t frameHeight,
    function<vector<uint8_t>(int32_t, int32_t)>* defaultAlg,
    function<vector<uint8_t>(int32_t, int32_t)>* customAlg)
{
    if(customAlg)
        return (*customAlg)(frameWidth, frameHeight);
    else
        return (*defaultAlg)(frameWidth, frameHeight);
}

int32_t StegaModule::Hide(uint8_t* frame, int32_t width, int32_t height, uint8_t* qrCode, int32_t qrwWidth){

}

int32_t StegaModule::GeneratePaths(int32_t frameWidth, int32_t frameHight, int32_t qrWidth){

    return 0;
}
