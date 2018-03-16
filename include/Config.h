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

void printEncCfg(Config cfg){
    string inFileName = cfg.m_ifName.size() ? cfg.m_ifName : string("<stdin>");
    string outFileName = cfg.m_ofName.size() ? cfg.m_ofName : string("<stdout>");
    string keyFileName = cfg.m_keyFileName.size() ? cfg.m_keyFileName : string("");

    LOG("Encoder settings: ");
    LOG("-i %s ", inFileName.c_str());
    LOG("-f %dx%d ", cfg.m_frameWidth, cfg.m_frameHeight);
    LOG("-a %d ", cfg.m_alignment);
    LOG("-o %s ", outFileName.c_str());
    LOG("-s %d ", cfg.m_qrScale);
    LOG("-e %d ", cfg.m_eccLevel);
    LOG("-t %d ", cfg.m_nTrailingFrames);
    LOG("-r %d ", cfg.m_frameRepeats);
    LOG("-w %d ", cfg.m_nWorkingThreads);
    LOG("-p %d ", cfg.m_framesPerThread);
    if(cfg.m_cipheringOn){
        LOG("-c %s ", keyFileName.c_str());
    }
    LOG("\nQR Version: %d ", cfg.m_qrVersion);
    LOG("\n");
}

void printDecCfg(Config cfg){
    string inFileName = cfg.m_ifName.size() ? cfg.m_ifName : string("<stdin>");
    string outFileName = cfg.m_ofName.size() ? cfg.m_ofName : string("<stdout>");
    string keyFileName = cfg.m_keyFileName.size() ? cfg.m_keyFileName : string("");

    LOG("Encoder settings: ");
    LOG("-i %s ", inFileName.c_str());
    LOG("-f %dx%d ", cfg.m_frameWidth, cfg.m_frameHeight);
    LOG("-o %s ", outFileName.c_str());
    LOG("-w %d ", cfg.m_nWorkingThreads);
    LOG("-p %d ", cfg.m_framesPerThread);
    LOG("-m %d ", cfg.m_decMode);
    if(cfg.m_skipDupFrames){
        LOG("-k ");
    }
    if(cfg.m_cipheringOn){
        LOG("-c %s ", keyFileName.c_str());
    }
    LOG("\n");
}

#endif // CONFIG_H
