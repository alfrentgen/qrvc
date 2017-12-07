#ifndef DECODE_H
#define DECODE_H
#include "pch.h"
#include "Job.h"
#include "inputQueue.h"
#include "OutputQueue.h"
#include <quirc.h>
#include <quirc_internal.h>
#include <zbar.h>

enum DecodeMode{
    QUICK = 0,
    MIXED = 1,
    SLOW = 2,
};

using namespace std;

class Decode : public Job
{
    public:
        Decode(int32_t fWidth, int32_t fHeight, InputQueue* inQ, OutputQueue* outQ, DecodeMode decMode);
        virtual ~Decode();
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
        zbar::ImageScanner m_scanner;
        zbar::Image m_image;
        struct quirc *m_qr;
        DecodeMode m_decMode;
        chrono::time_point<chrono::steady_clock> m_t1;
        chrono::time_point<chrono::steady_clock> m_t2;
        int32_t m_ID;

    protected:
        virtual uint32_t DecodeData();
        virtual uint32_t DecodeDataQuick();
        //uint32_t Decode::DecodeData_mock();

};

//#if USE_ZBAR
/*class DecodeQ : public Decode
{
    public:
        DecodeQ(int32_t fWidth, int32_t fHeight, InputQueue* inQ, OutputQueue* outQ);
        ~DecodeQ();

    private:
        struct quirc *m_qr;

    protected:
        virtual uint32_t DecodeData() override;

};*/
//#endif

#endif // DECODE_H
