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
        Encode(int32_t fWidth, int32_t fHeight, InputQueue* inQ, OutputQueue* outQ);
        virtual ~Encode();
        virtual int32_t Do() override;
        virtual void Stop() override;

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

    protected:
        virtual uint32_t EncodeData();
        //uint32_t Decode::DecodeData_mock();

};
#endif // ENCODE_H
