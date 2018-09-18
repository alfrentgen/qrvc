#ifndef STEGANOGRAPHY_H
#define STEGANOGRAPHY_H
#include <pch.h>

using namespace std;

#define RANDOM_PATH true
#define DEF_STEG_UNIT_SIZE 4
#define STEG_HIDE true
#define STEG_REVEAL false

typedef struct DWT{
        double std_dev;
        vector<uint32_t> buffer;
} DWT;

typedef struct StegUnit{

    bool useDWT;
    struct DWT dwt_aux;

    bool hide;
    uint8_t bit;
    uint8_t* pUnit;
    int32_t threshold;
    vector<int32_t> neighInds;
    vector<int32_t> coreInds;
    vector<uint8_t*> neighPels;
    vector<uint8_t*> corePels;
    StegUnit():
        bit(0), pUnit(nullptr), neighInds(0), coreInds(0), neighPels(0), corePels(0){}
} StegUnit_t;

class StegModule{
public:
    StegModule();
    int32_t Hide(uint8_t* frame, uint8_t* qrCode);
    int32_t Reveal(uint8_t* frame, uint8_t* qrCode);
    int32_t Process(uint8_t* frame, uint8_t* qrCode, bool action);
    int32_t Init(int32_t frameWidth, int32_t frameHeight, int32_t threshold, bool keyFlag = RANDOM_PATH);
    int32_t SetUnitPattern(char up);
    int32_t SetGenerator(int32_t val);
    int32_t ReadFramePath(string fileName);
    int32_t WriteFramePath(string fileName);
    int32_t GetQRWidth();
    int32_t SetCustomFramePath(uint8_t* path, uint32_t size);

    vector<int32_t> m_qrPath;
    vector<int32_t> m_framePath;
    bool m_keyFlag;
    int32_t m_frameWidth;
    int32_t m_frameHeight;
    int32_t m_qrWidth;
    int32_t m_maxQRWidth;
    int32_t m_threshold;
    char    m_unitPat;
    int32_t m_genVal;
    bool    m_useDWT;

private:
    vector<int32_t> m_coreIndeces;
    vector<int32_t> m_neighIndeces;
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
