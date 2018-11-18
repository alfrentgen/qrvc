#include "steganography.h"
#include "wavelet.h"

extern "C" {
    #include <qrencode.h>
    #include <qrspec.h>
    #include <qrinput.h>
}

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

    return path;
}

int32_t calc_mean(uint8_t** pixels, int32_t size){
    int32_t sum = 0;
    for(int i = 0; i < size; i++){
        sum += *pixels[i];
    }
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
    if(unit.hide){
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
    }else{
        if(meanCore - meanNeigh > 0){
            unit.bit = 255;
        }else{
            unit.bit = 0;
        }
    }

    return;
}

#define AVG_ACCURACY 8
inline int32_t getNewAvg(int32_t avg, int32_t bit, int32_t position){
    bit = bit ? 1 : 0;
    int32_t mask = 0xffffffff << position;
    int32_t trunkAvg = mask & avg;
    bool matched = (bit << position)&(trunkAvg) ? true : false;
    int32_t newAvg = trunkAvg;
    if(!matched){//use truncated
        newAvg = trunkAvg + (1 << position);
        if(newAvg > (255<<AVG_ACCURACY)){
            newAvg = trunkAvg - (1 << position);
        }
    }
    return newAvg;
}

void renderUnitAvg(StegUnit& unit){
    int32_t avg(0);
    for(uint8_t* pel : unit.corePels){
        avg+= *pel;
    }
    avg = (avg << AVG_ACCURACY)/unit.corePels.size();
    int32_t position = unit.bitPosition + AVG_ACCURACY;
    int32_t newAvg = getNewAvg(avg, unit.bit, position);
    int32_t totalDiff = (newAvg - avg) * unit.corePels.size();
    int32_t sign = totalDiff < 0 ? -1 : 1;
    totalDiff = abs(totalDiff);
    int32_t mask = 0x00000001 << (AVG_ACCURACY - 1);
    totalDiff += mask;
    totalDiff >>= AVG_ACCURACY;

    //the least first
    std::sort(unit.corePels.begin(), unit.corePels.end(),
                [](uint8_t* first, uint8_t* second)-> bool{
                    return *first < *second;
                });

    if(sign < 0){//the biggest first
        std::reverse(unit.corePels.begin(), unit.corePels.end());
    }

    uint8_t pel(0);
    int32_t room(0);
    int32_t delta(0);
    for(size_t i = unit.corePels.size() - 1; i >= 0; i--){//starting from the closest
        if(totalDiff <= 0){
            break;
        }
        pel = *unit.corePels[i];
        room = (sign > 0) ? 255 - pel : pel;
        if(room != 0){
            delta = totalDiff/(i+1);
            if(delta < totalDiff){//try to add one more
                delta++;
            }
            delta = (delta > room) ? room : delta;
            totalDiff -= delta;
            pel += sign*delta;
            *unit.corePels[i] = pel;
        }
    }
    return;
}
#undef AVG_ACCURACY

void fillUnitIndeces_Dot(int32_t stride, vector<int32_t>& core, vector<int32_t>& neigh){
        core.resize(0);
        neigh.resize(0);
        int32_t position(0);
        for(int row = 0; row < DEF_STEG_UNIT_SIZE; row++){
            for(int col = 0; col < DEF_STEG_UNIT_SIZE; col++){
                position = row * stride + col;
                core.push_back(position);
                neigh.push_back(position);
            }
        }
}

void fillUnitIndeces_O(int32_t stride, vector<int32_t>& core, vector<int32_t>& neigh){
        core = vector<int32_t>{stride + 1, stride + 2, 2*stride + 1, 2*stride + 2};
        neigh = vector<int32_t>{
        0, 1, 2, 3,//top border
        stride, 2*stride,//left border
        stride + 3, 2*stride + 3,//right border
        3*stride, 3*stride + 1, 3*stride + 2, 3*stride + 3 //bottom border
        };
}

void fillUnitIndeces_X(int32_t stride, vector<int32_t>& core, vector<int32_t>& neigh){

        core = vector<int32_t>{ 0, 1,                       //top left square
                                stride, stride + 1,
                                2*stride + 2, 2*stride + 3, //bottom right square
                                3*stride + 2, 3*stride + 3};

        neigh = vector<int32_t>{2, 3,                   //top right square
                                stride + 2, stride + 3,
                                2*stride, 2*stride + 1, //bottom left square
                                3*stride, 3*stride + 1};
}

void fillUnitIndeces_J(int32_t stride, vector<int32_t>& core, vector<int32_t>& neigh){

        core = vector<int32_t>{ 0, 1, stride, stride + 1//top left square
                                };

        neigh = vector<int32_t>{2, 3,                   //top right square
                                stride + 2, stride + 3,
                                2*stride, 2*stride + 1,//bottom left square
                                3*stride, 3*stride + 1,
                                2*stride + 2, 2*stride + 3,//bottom right square
                                3*stride + 2, 3*stride + 3
                                };
}

map<char, function<void(int32_t, vector<int32_t>&, vector<int32_t>&)>> unitIdxFill ={
{char('o'), function<void(int32_t, vector<int32_t>&, vector<int32_t>&)>(fillUnitIndeces_O)},
{char('x'), function<void(int32_t, vector<int32_t>&, vector<int32_t>&)>(fillUnitIndeces_X)},
{char('j'), function<void(int32_t, vector<int32_t>&, vector<int32_t>&)>(fillUnitIndeces_J)},
{char('.'), function<void(int32_t, vector<int32_t>&, vector<int32_t>&)>(fillUnitIndeces_Dot)}
};

int32_t StegModule::Hide(uint8_t* frame, uint8_t* qrCode){
    int32_t qrSize = m_qrWidth * m_qrWidth;
    int32_t stride = m_frameWidth;
    int32_t unitSize = DEF_STEG_UNIT_SIZE;
    StegUnit unit;
    unit.threshold = m_threshold;
    unit.corePels.resize(m_coreIndeces.size(), nullptr);
    unit.neighPels.resize(m_neighIndeces.size(), nullptr);

    for(int i = 0; i < qrSize; i++){
        int32_t qrIdx = m_qrPath[i];
        int32_t x = m_framePath[2*i];
        int32_t y = m_framePath[2*i+1];
        unit.bit = qrCode[qrIdx];
        unit.pUnit = frame + y * unitSize * stride + x * unitSize;

        for(int32_t j = 0; j < m_coreIndeces.size(); j++){
            unit.corePels[j] = unit.pUnit + m_coreIndeces[j];
        }

        for(int32_t j = 0; j < m_neighIndeces.size(); j++){
            unit.neighPels[j] = unit.pUnit + m_neighIndeces[j];
        }
        unit.hide = STEG_HIDE;
        renderUnit(unit);
    }
    return OK;
}

int32_t StegModule::Reveal(uint8_t* frame, uint8_t* qrCode){
    int32_t qrSize = m_qrWidth * m_qrWidth;
    int32_t stride = m_frameWidth;
    int32_t unitSize = DEF_STEG_UNIT_SIZE;
    StegUnit unit;
    unit.corePels.resize(m_coreIndeces.size(), nullptr);
    unit.neighPels.resize(m_neighIndeces.size(), nullptr);

    for(int i = 0; i < qrSize; i++){
        int32_t qrIdx = m_qrPath[i];
        int32_t x = m_framePath[2*i];
        int32_t y = m_framePath[2*i+1];
        unit.pUnit = frame + y * unitSize * stride + x * unitSize;

        for(int32_t j = 0; j < m_coreIndeces.size(); j++){
            unit.corePels[j] = unit.pUnit + m_coreIndeces[j];
        }
        for(int32_t j = 0; j < m_neighIndeces.size(); j++){
            unit.neighPels[j] = unit.pUnit + m_neighIndeces[j];
        }
        unit.hide = STEG_REVEAL;
        renderUnit(unit);
        qrCode[qrIdx] = unit.bit;
    }
    return OK;
}

int32_t StegModule::Process(uint8_t* frame, uint8_t* qrCode, bool action){
    int32_t qrSize = m_qrWidth * m_qrWidth;
    int32_t stride = m_frameWidth;
    int32_t unitSize = DEF_STEG_UNIT_SIZE;
    StegUnit unit;
    unit.threshold = m_threshold;//for HIDE
    unit.corePels.resize(m_coreIndeces.size(), nullptr);
    unit.neighPels.resize(m_neighIndeces.size(), nullptr);

    for(int i = 0; i < qrSize; i++){
        int32_t qrIdx = m_qrPath[i];
        int32_t x = m_framePath[2*i];
        int32_t y = m_framePath[2*i+1];
        unit.bit = qrCode[qrIdx];//for HIDE
        unit.pUnit = frame + y * unitSize * stride + x * unitSize;

        for(int32_t j = 0; j < m_coreIndeces.size(); j++){
            unit.corePels[j] = unit.pUnit + m_coreIndeces[j];
        }

        for(int32_t j = 0; j < m_neighIndeces.size(); j++){
            unit.neighPels[j] = unit.pUnit + m_neighIndeces[j];
        }
        unit.hide = action;
        if(m_useAvg){
            renderUnitAvg(unit);
        }else{
            renderUnit(unit);
        }
        qrCode[qrIdx] = unit.bit;//for REVEAL
    }
    return OK;
}

//return 0 if no versions can fit current frame path length
int32_t calcMaxQRWidth(int32_t dotsAvailable){
    int32_t maxWidth = floor(sqrt(dotsAvailable));
    int32_t version = QRSPEC_VERSION_MAX;
    int32_t width;

    do{
        width = QRspec_getWidth(version--);
    }while(width > maxWidth);

    return width;
}

//returns -1 if qrPath is longer than framePath,
//because there are not enough units in a frame to hide each dot of QR code
int32_t StegModule::Init(int32_t frameWidth, int32_t frameHeight, int32_t threshold, bool keyFlag){
    m_frameWidth = frameWidth;
    m_frameHeight = frameHeight;
    m_keyFlag = keyFlag;
    m_threshold = threshold;

    m_maxQRWidth = calcMaxQRWidth((m_frameHeight / DEF_STEG_UNIT_SIZE) * (m_frameWidth / DEF_STEG_UNIT_SIZE));
    if(m_maxQRWidth == 0){
        return FAIL;
    }

    function<vector<int32_t>(int32_t, int32_t, bool)> defFramePathGen(generateDefaultFramePath);
    m_framePath = generateFramePath(m_frameWidth, m_frameHeight, m_keyFlag, &defFramePathGen, nullptr);
    m_qrWidth = calcMaxQRWidth(m_framePath.size()/2);
    m_qrPath = generateQRPath(m_qrWidth, nullptr);

    if(SetUnitPattern(m_unitPat) != OK){
        return FAIL;
    }
    return OK;
}

int32_t StegModule::GetQRWidth(){
    return m_qrWidth;
}

#define TO_BYTES true
#define TO_INT false

void getLEInt(uint32_t& intVal, uint8_t leInt[4], bool conv){
    if(conv == TO_BYTES){
        for(int i = 0; i < 4; i++){
            leInt[i] = (uint8_t)((intVal >> i*8) & 0xff);
        }
    }else{
        intVal = 0;
        for(int i = 0; i < 4; i++){
            intVal |= ((uint32_t)leInt[i] << i*8);
        }
    }
}

#define COORD_SIZE sizeof(uint32_t)

int32_t StegModule::ReadFramePath(string fileName){
    ifstream ifs(fileName, ios_base::in | ios_base::binary);
    vector<uint8_t> framePathBytes(0);
    if(ifs.bad()){
        LOG("Cannot read steganography key file!\n");
        return FAIL;
    }
    while (ifs.good()) {
        framePathBytes.push_back((uint8_t)ifs.get());
    }

    int32_t nPositions = framePathBytes.size()/(COORD_SIZE * 2);
    int32_t qrWidth = calcMaxQRWidth(nPositions);
    if(qrWidth == 0){
        LOG("Steganography key file is too small!\n");
        return FAIL;
    }else{
        LOG("Steg key file reading results: %d dots read, QR width = %d\n", nPositions, qrWidth);
    }

    SetCustomFramePath(framePathBytes.data(), nPositions * (2 * COORD_SIZE));
    m_qrWidth = qrWidth;
    m_qrPath = generateQRPath(m_qrWidth, nullptr);
    return OK;
}

int32_t StegModule::SetCustomFramePath(uint8_t* pathBytes, uint32_t size){
    int32_t nValues = size / COORD_SIZE;
    m_framePath.resize(0);
    uint32_t val;
    for(uint32_t i = 0; i < nValues; i++){
        getLEInt(val, pathBytes + i * COORD_SIZE, TO_INT);
        m_framePath.push_back(val);
    }
    return OK;
}

int32_t StegModule::WriteFramePath(string fileName){
    string keyFileName = fileName;
    ofstream ofs(keyFileName, ios_base::out | ios_base::binary);
    if(ofs.bad()){
        LOG("Cannot write steganography key file \"%s\"! Terminate!\n", keyFileName.c_str());
        ofs.close();
        return FAIL;
    }
    vector<uint8_t> framePathSerialized(0);
    uint32_t qrSize = m_qrWidth * m_qrWidth;
    uint32_t coordsToWrite = 2 * qrSize;
    coordsToWrite = coordsToWrite > m_framePath.size() ? m_framePath.size() : coordsToWrite;
    uint8_t leInt[4] = {0};
    for(int32_t i = 0; i < coordsToWrite; i++){
        uint32_t intVal = m_framePath[i];
        getLEInt(intVal, leInt, TO_BYTES);
        framePathSerialized.insert(framePathSerialized.end(), leInt, leInt + 4);
    }
    ofs.write(framePathSerialized.data(), framePathSerialized.size());
    ofs.close();
    return OK;
}

int32_t StegModule::SetUnitPattern(char up){
    auto it = unitIdxFill.find(up);
    if(it != unitIdxFill.end()){
        it->second(m_frameWidth, m_coreIndeces, m_neighIndeces);
        m_unitPat = up;
    }else{
        return FAIL;
    }
    return OK;
}

int32_t StegModule::SetGenerator(int32_t val){
    if(val < -1 || val > 255){
        return FAIL;
    }
    m_genVal = val;
    return OK;
}

StegModule::StegModule() :
m_unitPat('o'),
m_useAvg(false)
{
}
