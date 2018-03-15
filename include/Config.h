#ifndef CONFIG_H
#define CONFIG_H

#include "pch.h"

#define ECC_LEVEL_L 0
#define ECC_LEVEL_M 1
#define ECC_LEVEL_H 2
#define ECC_LEVEL_Q 3

#define MODE_QUICK 0
#define MODE_MIXED 1
#define MODE_SLOW 2

using namespace std;

typedef struct Config {
    bool    m_cipheringOn;
    bool    m_inverseFrame;
    bool    m_skipDupFrames;

    int32_t m_frameWidth;
    int32_t m_frameHeight;
    int32_t m_frameRepeats;
    int32_t m_nTrailingFrames;

    int32_t m_nWorkingThreads;
    int32_t m_framesPerThread;

    int32_t m_qrScale;
    int32_t m_eccLevel;
    int32_t m_qrVersion;
    int32_t m_alignment;

    int32_t m_decMode;

    string  m_ifName;
    string  m_ofName;
    string  m_keyFileName;

    Config() :
        m_frameWidth(1280),
        m_frameHeight(720),
        m_cipheringOn(false),
        m_frameRepeats(1),
        m_framesPerThread(8),
        m_nWorkingThreads(0),
        m_nTrailingFrames(0),
        m_qrVersion(0),
        m_qrScale(4),
        m_eccLevel(ECC_LEVEL_L),
        m_inverseFrame(false),
        m_decMode(MODE_MIXED),
        m_skipDupFrames(false),
        m_ifName(),
        m_ofName(),
        m_keyFileName()
    {}

} Config;
#endif // CONFIG_H
