#ifndef ENCODE_H
#define ENCODE_H
#include "pch.h"
#include "Job.h"
#include "inputQueue.h"
#include "OutputQueue.h"
#include "Config.h"
#include "steganography.h"

extern "C" {
    #include <qrencode.h>
    #include <qrspec.h>
    #include <qrinput.h>
}

using namespace std;

class Encode : public Job
{
    public:
        Encode(Config& config, InputQueue* inQ, OutputQueue* outQ);
        virtual ~Encode();
        virtual int32_t Do() override;
        virtual void Stop() override;
        void SetCypheringParams(vector<uint8_t>* pKeyFrame, ofstream* pKeyFileOS);
        void SetStegParams(StegModule* stegModule);

    protected:
        uint32_t m_frameWidth;
        uint32_t m_frameHeight;
        InputQueue* m_inQ;
        OutputQueue* m_outQ;
        Chunk m_data;
        atomic_flag m_isWorking;

    private:
        chrono::time_point<chrono::steady_clock> m_t1;
        chrono::time_point<chrono::steady_clock> m_t2;
        int32_t m_ID;
        QRecLevel m_eccLevel;
        int32_t m_version;
        int32_t m_qrScale;
        bool m_invertColors;
        int32_t m_frameRepeats;
        int32_t m_tailSize;
        int32_t m_alignment;
        vector<uint8_t>* m_pKey;
        ofstream* m_pKeyFileStream;
        StegModule* m_stegModule;

    protected:
        virtual uint32_t EncodeData();

    private:
        void FillFrames(vector<uint8_t>& frames, int32_t frameSize, int32_t xOffset, int32_t yOffset, int32_t frameRepeats,
                        uint8_t* pQRData, int32_t qrWidth, int32_t drawColor, int32_t bgColor);

};
#endif // ENCODE_H
