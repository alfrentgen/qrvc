#ifndef DECODE_H
#define DECODE_H
#include "pch.h"
#include "Job.h"
#include "inputQueue.h"
#include "OutputQueue.h"
#include <quirc.h>
#include <quirc_internal.h>
#include <zbar.h>
#include "Config.h"
#include "steganography.h"

using namespace std;

class Decode : public Job
{
    public:
        Decode(Config& config, InputQueue* inQ, OutputQueue* outQ);
        virtual ~Decode();
        virtual int32_t Do() override;
        virtual void Stop() override;
        void SetCypheringParams(vector<uint8_t>* pKeyFrame);
        void SetStegParams(StegModule* stegModule);

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
        int32_t m_decMode;
        int32_t m_ID;
        bool m_skipDup;
        vector<uint8_t>* m_pKeyFrame;
        StegModule* m_stegModule;

        chrono::time_point<chrono::steady_clock> m_t1;
        chrono::time_point<chrono::steady_clock> m_t2;


    protected:
        virtual uint32_t DecodeData();
        virtual uint32_t DecodeDataQuick();
        //virtual uint32_t DecodeDataSteg();

    private:
        uint32_t ExtractHashsum();
        uint64_t ExtractChunkID();
};

#endif // DECODE_H
