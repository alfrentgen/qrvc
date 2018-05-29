#include "steganography.h"

#define DEF_STEG_UNIT_SIZE 4

vector<int32_t> spiralMatrix(int32_t qrWidth){
    //currentIdx
    vector<int32_t> indeces(0);

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

vector<int32_t> generateDefaultQRPath(int32_t qrWidth){
    return spiralMatrix(qrWidth);
}

vector<int32_t> generateQRPath(int32_t qrWidth, function<vector<int32_t>(int32_t)>* customAlg){
    if(customAlg)
        return (*customAlg)(qrWidth);
    else
        return generateDefaultQRPath(qrWidth);
}

int32_t getUnitsCount(int32_t size){
    int32_t units = size/DEF_STEG_UNIT_SIZE - 1;

    int32_t borderSize = size%DEF_STEG_UNIT_SIZE;
    if(borderSize == 0){
        units -= 1;
    }
    return units;
}

vector<int32_t> generateDefaultFramePath(int32_t frameWidth, int32_t frameHeight, bool keyFlag){
    vector<int32_t> path(0);
    //left and top borders should be at least 4 pels sized
    //right and bottom borders should be not zero
    //A steg unit is 4x4 pels
    //border pixels(units) must not be used
    int32_t unitsX = getUnitsCount(frameWidth);
    int32_t unitsY = getUnitsCount(frameHeight);

    vector<int32_t> xIndeces(unitsX);
    vector<int32_t> yIndeces(unitsY);

    for(int i = 1; i <= unitsX; i++){
        xIndeces[i] = i;
    }

    for(int i = 1; i <= unitsY; i++){
        yIndeces[i] = i;
    }

    if(keyFlag){
            //Just generating random  matrix
            std::random_shuffle(xIndeces.begin(), xIndeces.end());
            std::random_shuffle(yIndeces.begin(), yIndeces.end());
    }

    int32_t nTaps = unitsX < unitsY ? unitsX : unitsY;
    path.resize(2 * nTaps);
    for(int i = 0; i < nTaps; i++){
        path[2 * i] = xIndeces[i];
        path[2 * i + 1] = yIndeces[i];
    }

    return path;
}

//tuple<raw, column>
vector<int32_t> generateFramePath(int32_t frameWidth, int32_t frameHeight, bool keyFlag,
    function<vector<int32_t>(int32_t, int32_t, bool)>* defaultAlg,
    function<vector<int32_t>(int32_t, int32_t, bool)>* customAlg)
{
    if(customAlg)
        return (*customAlg)(frameWidth, frameHeight, keyFlag);
    else
        return (*defaultAlg)(frameWidth, frameHeight, keyFlag);
}

int32_t calc_mean(uint8_t* pixels, int32_t size){
    int32_t sum = 0;
    for(int i = 0; i < size; i++)
        sum += pixels[i];
    return sum/size;
}

//4x4 pels unit
void changeUnit(StegUnit& unit){
    int8_t inc = 1;
    if(unit.bit){
        inc = -1;
    }

    /*vector<uint8_t*> pCorePels = {unit};
    int32_t meanCore = calc_mean();
    int32_t meanNeigh = calc_mean();
    for(){}*/

    /*#neighbourpels
    for i in range(0, len(neigh)):
        if (neigh[i] < 255) and(neigh[i] > 0):
            neigh[i] = neigh[i] + inc
    for()

    #selected pel
    for i in range(0, len(pels)):
        if (pels[0] < 255) and(pels[0] > 0):
                pels[i] = pels[i] - inc*/

    return;
}

int32_t StegaModule::Hide(uint8_t* frame, uint8_t* qrCode){
    int32_t qrSize = m_qrWidth * m_qrWidth;
    int32_t stride = m_frameWidth;
    int32_t unitSize = DEF_STEG_UNIT_SIZE;
    StegUnit unit;
    unit.threshold = m_threshold;
    unit.neighInds = vector<int32_t>{
        0, 1, 2, 3,//top border
        stride, 2*stride,//left border
        stride + unitSize - 1, 2*stride + unitSize - 1,//right border
        3*stride, 3*stride + 1, 3*stride + 2, 3*stride + 3 //bottom border
        };
    unit.coreInds = {stride + 1, stride + 2, 2*stride + 1, 2*stride + 2};

    for(int i = 0; i < qrSize; i++){
        int32_t qrIdx = m_qrPath[i];
        int32_t unitPosX = m_framePath[2 * i];
        int32_t unitPosY = m_framePath[2 * i + 1];

        uint8_t* pUnit = &frame[unitPosY * unitSize * stride + unitPosX * unitSize];
        uint8_t qrDot = qrCode[qrIdx];
        unit.bit = qrDot;
        unit.pUnit = pUnit;

        changeUnit(unit);
    }
}

//returns -1 if qrPath is longer than framePath,
//because there are not enough units in a frame to hide each dot of QR code
int32_t StegaModule::Init(int32_t frameWidth, int32_t frameHeight, int32_t qrWidth, int32_t threshold, bool keyFlag){
    m_frameWidth = frameWidth;
    m_frameHeight = frameHeight;
    m_qrWidth = qrWidth;
    m_keyFlag = keyFlag;
    m_threshold = threshold;

    function<vector<int32_t>(int32_t, int32_t, bool)> defFramePathGen(generateDefaultFramePath);
    m_framePath = generateFramePath(m_frameWidth, m_frameHeight, m_keyFlag, &defFramePathGen, nullptr);
    m_qrPath = generateQRPath(m_qrWidth, nullptr);
    if(m_qrPath.size() > m_framePath.size())
        return FAIL;
    return OK;
}
