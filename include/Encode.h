#ifndef ENCODE_H
#define ENCODE_H
#include "pch.h"
#include "Job.h"
#include "inputQueue.h"
#include "OutputQueue.h"

extern "C" {
    #include <qrencode.h>
    #include <qrspec.h>
    #include <qrinput.h>
}

using namespace std;

class Encode : public Job
{
    public:
        Encode(int32_t fWidth, int32_t fHeight, int32_t frameRepeats, int32_t tailSize, bool invert,
                InputQueue* inQ, OutputQueue* outQ, int32_t version, QRecLevel eccLevel, int32_t qrScale, int32_t alignment);
        virtual ~Encode();
        virtual int32_t Do() override;
        virtual void Stop() override;
        void SetCypheringParams(vector<uint8_t>* pKeyFrame, ofstream* pKeyFileOS);

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
        vector<uint8_t>* m_pKeyFrame;
        ofstream* m_pKeyFileStream;

    protected:
        virtual uint32_t EncodeData();
        //uint32_t Decode::DecodeData_mock();

};
#endif // ENCODE_H
