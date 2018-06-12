#include "steganography.h"

#define DEF_STEG_UNIT_SIZE 4

vector<int32_t> Matrix2D(int32_t qrWidth){
    vector<int32_t> indeces(0);
    for(int i = 0; i < qrWidth; i++){
        for(int j = 0; j < qrWidth; j++){
            indeces.push_back(i* qrWidth + j);
        }
    }
    return indeces;
}

vector<int32_t> spiralMatrix(int32_t qrWidth){
    //currentIdx
    vector<int32_t> indeces(0);

    int32_t sideLength = qrWidth;
    int32_t index = 0;
    int32_t stride = 1;

    while(sideLength > 0){
        for(int i = 0; i < 4; i++){
            int32_t direction = (i >= 2) ? -1 : 1;
            stride = i%2 ? qrWidth : 1;
            int32_t diff = direction * stride;

            for(int j = 0; j < sideLength-1; j++){
                indeces.push_back(index);
                index += diff;
            }
        }
        index += stride + 1;
        sideLength -= 2;
    }
    if(sideLength == 1){
        index -= (stride + 1);
        indeces.push_back(index);
    }
    return indeces;
}

vector<int32_t> generateDefaultQRPath(int32_t qrWidth){
    //return Matrix2D(qrWidth);
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

vector<int32_t> old_generateDefaultFramePath(int32_t frameWidth, int32_t frameHeight, bool keyFlag){
    //vector<int32_t> path(0);
    //left and top borders should be at least 4 pels sized
    //right and bottom borders should be not zero
    //A steg unit is 4x4 pels
    //border pixels(units) must not be used


    int32_t unitsX = getUnitsCount(frameWidth);
    int32_t unitsY = getUnitsCount(frameHeight);

    vector<int32_t> xIndeces(unitsX);
    vector<int32_t> yIndeces(unitsY);

    for(int i = 0; i < unitsX; i++){
        xIndeces[i] = i + 1;
    }

    for(int i = 0; i < unitsY; i++){
        yIndeces[i] = i + 1;
    }

    if(keyFlag){
            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            //Just generating random  matrix
            std::shuffle(xIndeces.begin(), xIndeces.end(), std::default_random_engine(seed));
            std::shuffle(yIndeces.begin(), yIndeces.end(), std::default_random_engine(seed));
            //std::random_shuffle(xIndeces.begin(), xIndeces.end());
            //std::random_shuffle(yIndeces.begin(), yIndeces.end());
    }

    int32_t nTaps = unitsX < unitsY ? unitsX : unitsY;
    cout << nTaps << endl;
    vector<int32_t> path(2*nTaps*nTaps, 0);

    for(int i = 0; i < nTaps; i++){//raw
        for(int j = 0; j < nTaps; j++){//column
            path[2 * i * nTaps + 2 * j] = xIndeces[i];
            path[2 * i * nTaps + 2 * j + 1] = yIndeces[j];
            //LOG("%d, %d\n", path[i * 2 * nTaps + 2 * j], path[i * 2 * nTaps + 2 * j + 1]);
        }
    }
    /*for(int i = 0; i < nTaps; i++){
        for(int j = 0; j < nTaps; j++){
            LOG("%d, %d\n", path[i * 2 * nTaps + 2 * j], path[i * 2 * nTaps + 2 * j + 1]);
        }
    }*/
    /*for(int32_t idx : path){
        LOG("%d,", idx);
    }*/
    LOG("defGenPATH\n");

    return path;
}

vector<int32_t> generateDefaultFramePath(int32_t frameWidth, int32_t frameHeight, bool keyFlag){
    //vector<int32_t> path(0);
    //left and top borders should be at least 4 pels sized
    //right and bottom borders should be not zero
    //A steg unit is 4x4 pels
    //border pixels(units) must not be used
    int32_t unitsX = frameWidth / DEF_STEG_UNIT_SIZE;
    int32_t unitsY = frameHeight / DEF_STEG_UNIT_SIZE;
    int32_t nUnits = (unitsX - 2) * (unitsY - 2);//subtract left, right, top and bottom borders

    vector<int32_t> indeces(nUnits);

    int n = 0;
    for(int y = 1; y < unitsY - 1; y++){
        for(int x = 1; x < unitsX - 1; x++){
            indeces[n] = y * unitsX + x;
            n++;
        }
    }

    if(keyFlag){
            //Just generating random  matrix
            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            std::shuffle(indeces.begin(), indeces.end(), std::default_random_engine(seed));
    }


    cout << nUnits << endl;
    vector<int32_t> path(2 * nUnits, 0);

    for(int i = 0; i < indeces.size(); i++){
        path[2 * i] = indeces[i] % unitsX;
        path[2 * i + 1] = indeces[i] / unitsX;
    }

    return path;
}

//tuple<raw, column>
vector<int32_t> generateFramePath(int32_t frameWidth, int32_t frameHeight, bool keyFlag,
    function<vector<int32_t>(int32_t, int32_t, bool)>* defaultAlg,
    function<vector<int32_t>(int32_t, int32_t, bool)>* customAlg)
{
    function<vector<int32_t>(int32_t, int32_t, bool)>& alg = customAlg ? *customAlg : *defaultAlg;
    vector<int32_t> path = alg(frameWidth, frameHeight, keyFlag);
    /*for(int32_t idx : path){
        LOG("%d,", idx);
    }
    LOG("genFramePATH\n");*/

    return path;
}

int32_t calc_mean(uint8_t** pixels, int32_t size){
    int32_t sum = 0;
    for(int i = 0; i < size; i++)
        sum += *pixels[i];
    return sum/size;
}

void changePels(uint8_t** pels, int32_t nPels, int32_t diff){
    int32_t absDiff = abs(diff);
    for(int32_t i = 0; i < nPels; i++){
        if(diff < 0 && *pels[i] >=  absDiff){
            *pels[i] += diff;
        }else if(diff > 0 && *pels[i] < (255 - absDiff)) {
            *pels[i] += diff;
        }
    }
}

//4x4 pels unit
void renderUnit(StegUnit& unit){

    int32_t meanCore = calc_mean(unit.corePels.data(), unit.corePels.size());
    int32_t meanNeigh = calc_mean(unit.neighPels.data(), unit.neighPels.size());
    int8_t diff = 1;
    int8_t dir = 1;
    if(unit.bit == 0){
        dir = -1;
    }
    diff *= dir;
    //LOG("diff=%d, meanNeigh=%d\n", unit.bit, meanNeigh);
    while(dir*(meanCore - meanNeigh) <= unit.threshold){
            changePels(unit.corePels.data(), unit.corePels.size(), diff);
            changePels(unit.neighPels.data(), unit.neighPels.size(), -diff);
            meanCore = calc_mean(unit.corePels.data(), unit.corePels.size());
            meanNeigh = calc_mean(unit.neighPels.data(), unit.neighPels.size());
            //LOG("unit.threshold=%d\n", unit.threshold);
            //LOG("meanCore=%d, meanNeigh=%d\n", meanCore, meanNeigh);
    }

    /*if(unit.bit == 0){
        //while(meanNeigh - meanCore <= unit.threshold){
        while(dir*(meanCore - meanNeigh) <= unit.threshold){
            changePels(unit.corePels.data(), unit.corePels.size(), diff);
            changePels(unit.neighPels.data(), unit.neighPels.size(), -diff);
            meanCore = calc_mean();
            meanNeigh = calc_mean();
        }
    }
    else{
        //while(meanNeigh - meanCore >= -unit.threshold){
        while(dir*(meanCore - meanNeigh) <= unit.threshold){
            changePels(unit.corePels.data(), unit.corePels.size(), diff);
            changePels(unit.neighPels.data(), unit.neighPels.size(), -diff);
            meanCore = calc_mean();
            meanNeigh = calc_mean();
        }
    }*/

    return;
}

int32_t StegModule::Hide(uint8_t* frame, uint8_t* qrCode){
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

    unit.corePels.resize(unit.coreInds.size(), nullptr);
    unit.neighPels.resize(unit.neighInds.size(), nullptr);

    for(int i = 0; i < qrSize; i++){
        //LOG("%d\n",i);
        int32_t qrIdx = m_qrPath[i];
        int32_t unitPosX = m_framePath[2 * i];
        int32_t unitPosY = m_framePath[2 * i + 1];
        //LOG("%d, %d, %d,%d\n", unitPosX, unitPosY, qrIdx, qrCode[qrIdx]);

        uint8_t* pUnit = &frame[unitPosY * unitSize * stride + unitPosX * unitSize];
        uint8_t qrDot = qrCode[qrIdx];
        unit.bit = qrDot;
        unit.pUnit = pUnit;
        for(int32_t i = 0; i < unit.coreInds.size(); i++){
            unit.corePels[i] = unit.pUnit + unit.coreInds[i];
        }

        for(int32_t i = 0; i < unit.neighInds.size(); i++){
            unit.neighPels[i] = unit.pUnit + unit.neighInds[i];
        }
        //LOG("Rendering Unit #%d\n",i);
        /*for(auto idx : unit.coreInds){
            LOG("%d, ", idx);
        }
        LOG("\n");
        for(auto idx : unit.neighInds){
            LOG("%d, ", idx);
        }
        LOG("\n");*/
        renderUnit(unit);
    }
}

//returns -1 if qrPath is longer than framePath,
//because there are not enough units in a frame to hide each dot of QR code
int32_t StegModule::Init(int32_t frameWidth, int32_t frameHeight, int32_t qrWidth, int32_t threshold, bool keyFlag){
    m_frameWidth = frameWidth;
    m_frameHeight = frameHeight;
    m_qrWidth = qrWidth;
    m_keyFlag = keyFlag;
    m_threshold = threshold;

    function<vector<int32_t>(int32_t, int32_t, bool)> defFramePathGen(generateDefaultFramePath);
    m_framePath = generateFramePath(m_frameWidth, m_frameHeight, m_keyFlag, &defFramePathGen, nullptr);
    m_qrPath = generateQRPath(m_qrWidth, nullptr);
    LOG("%d\t%d\n", m_qrPath.size(), m_framePath.size());
    if(2 * m_qrPath.size() > m_framePath.size()){
        return FAIL;
    }
    return OK;
}
